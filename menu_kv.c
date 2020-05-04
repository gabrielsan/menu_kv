#include <string.h>
#include <stdint.h>

#include "TimerOne.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 12 // Lower resolution

// LCD
#define LCD_PIN_RS 10
#define LCD_PIN_EN 11
#define LCD_PIN_D4 12
#define LCD_PIN_D5 13
#define LCD_PIN_D6 A0
#define LCD_PIN_D7 A1

// BUTTONS
#define BT_UP_PIN     7
#define BT_DOWN_PIN   8
#define BT_ENTER_PIN  6
#define BT_CANCEL_PIN  9

#define BT_PRESS 0x1
#define BT_HOLD 0x2
#define BT_EVENT 0x8000
#define BT_UP		0x01
#define BT_DOWN		0x02
#define BT_ENTER	0x04
#define BT_CANCEL	0x08


// outputs

#define TIMERONE_TICK_US 50000
#define TIMERONE_TICK_MS (TIMERONE_TICK_US / 1000)
#define LOOP_DELAY_MS 50

// HEATER
#define HLT_HEATER_PIN  5
#define BOIL_HEATER_PIN 3

// MENU
#define MENU_FLAGS_EDITING 0x40

// PARAM
#define PARAM_OP_WCONFIRM 0x08
#define PARAM_OP_DEC0 0x10
#define PARAM_OP_DEC1 0x20
#define PARAM_OP_DEC2 0x40
#define PARAM_OP_DEC3 0x80
#define PARAM_OP_RDONLY 0x800
#define PARAM_IS_RDONLY(param) ((param->options & PARAM_OP_RDONLY) != 0)

#define PARAM_SALLOC_INT16(namee, initval, op, minval, maxval, inc) \
static struct param_st p_##namee = { \
	.value = (uint16_t)initval, \
	.options = op, \
	.minValue = minval, \
	.maxValue = maxval, \
	.increment = inc, \
};

// TEMPERATURE SENSORS
#define SENSOR_DS18B20_ADDR_0 0x28FFA45B00160130
#define SENSOR_DS18B20_ADDR_1 0x28FF18B1001602EE
#define SENSOR_DS18B20_ADDR_2 0x28FF60A600160203

enum menuEvent_en {
	MNEV_UP,
	MNEV_DOWN,
	MNEV_ENTER,
	MNEV_CANCEL,
	MNEV_NONE,
};

#define TTFLG_ON 0x1
#define TTFLG_UNDERFLOWED 0x2

struct tickTimer_st {
	uint32_t tickToCnt;
	int16_t timeMin;
	uint16_t flags;
};

struct bt_st {
	uint32_t tick100ms;
	uint8_t pin;
	uint8_t holdstep;
	uint8_t reserved[2]; // align(4)
};

struct param_st {
	int16_t value;
	const uint16_t options;
	const int16_t minValue;
	const int16_t maxValue;
	const int16_t increment;
};

struct menu_st {
	const struct menuItem_st *list;	
	const uint8_t msize;
	uint8_t pos;
	uint8_t cursorPos;
	uint8_t flags;
	int16_t paramcpy;
};

enum menuFormat_en {
	MFORMAT_PARAM,
	MFORMAT_APP,
};

struct menuItem_st {
	const char *desc;
	void *mArg;
	enum menuFormat_en mFormat;
};

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire); 
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_EN, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);
// buttons ctx
static struct bt_st btup, btdown, btenter, btcancel;
// global tick
static volatile uint32_t g_tick100ms = 0;
static volatile uint32_t g_tick50ms = 0;

#define SENSOR_DS18B20_HLT                       0x28FFA45B00160130
#define SENSOR_DS18B20_CHILLER                   0x28FF60A600160203
#define SENSOR_DS18B20_BOIL                      0x28FF5BE9C21603CF
DeviceAddress tempSensorAddr_HLT     = { 0x28, 0xFF, 0xA4, 0x5B, 0x00, 0x16, 0x01, 0x30 };
DeviceAddress tempSensorAddr_BOIL = { 0x28, 0xFF, 0x60, 0xA6, 0x00, 0x16, 0x02, 0x03 };
DeviceAddress tempSensorAddr_CHILLER    = { 0x28, 0xFF, 0x5B, 0xE9, 0xC2, 0x16, 0x03, 0xCF };

// parameters
// Temperature read for Hot Liquor Tank (Line) kettle sensor
PARAM_SALLOC_INT16(hltLineTempRead, 0, PARAM_OP_DEC1 | PARAM_OP_RDONLY, -200, 1500, 1)

PARAM_SALLOC_INT16(chillerTempRead, 0, PARAM_OP_DEC1 | PARAM_OP_RDONLY, -200, 1500, 1)
// Temperature setting for HLT kettle
PARAM_SALLOC_INT16(hltTempSet, 0, PARAM_OP_DEC1, -200, 1500, 5)
// Hysteresis Temperature setting for HLT kettle
PARAM_SALLOC_INT16(hltTempDelta, 0, PARAM_OP_DEC1, -200, 1500, 1)
PARAM_SALLOC_INT16(boilTempDelta, 0, PARAM_OP_DEC1, -200, 1500, 1)
// Turn on timer setting for HLT
PARAM_SALLOC_INT16(hltOn, 0, 0, 0, 1, 1)

PARAM_SALLOC_INT16(boilOn, 0, 0, 0, 1, 1)
// Turn on timer setting for HLT
PARAM_SALLOC_INT16(hltTime2Cook, 0, 0, 0, 1000, 10)
PARAM_SALLOC_INT16(hltCookTimeLeft, 0, PARAM_OP_RDONLY, 0, 1000, 10)

// Temperature read for Boil kettle sensor
PARAM_SALLOC_INT16(boilTempRead, 0, PARAM_OP_DEC1 | PARAM_OP_RDONLY, -200, 1500, 1)
// Temperature setting for Boil kettle
PARAM_SALLOC_INT16(boilTempSet, 0, PARAM_OP_DEC1, -200, 1500, 5)
// Hysteresis Temperature setting for Boil kettle
//PARAM_SALLOC_INT16(boilTempHyst, 0, PARAM_OP_DEC1, -200, 1500, 1)
// Heater power output for HLT kettle
//PARAM_SALLOC_INT16(hltHeaterPower, 0, PARAM_OP_RDONLY, 0, 100, 10)
// Heater power output for Boil kettle
//PARAM_SALLOC_INT16(boilHeaterPower, 0, PARAM_OP_RDONLY, 0, 100, 10)

// main menu list
static const struct menuItem_st gouglaszMenuList[] = {
	{.desc = "TEMP SENSOR",	.mArg = (void *)&p_hltLineTempRead,	.mFormat = MFORMAT_PARAM},
	{.desc = "TEMP SET",	  .mArg = (void *)&p_hltTempSet,   	  .mFormat = MFORMAT_PARAM},
	{.desc = "TEMP DELTA",	.mArg = (void *)&p_hltTempDelta,	  .mFormat = MFORMAT_PARAM},
	{.desc = "TEMP EN.",   	.mArg = (void *)&p_hltOn, 			    .mFormat = MFORMAT_PARAM},
	{.desc = "TIME2COOK", 	.mArg = (void *)&p_hltTime2Cook,    .mFormat = MFORMAT_PARAM},
  {.desc = "COOKT.LEFT",  .mArg = (void *)&p_hltCookTimeLeft, .mFormat = MFORMAT_PARAM},
  {.desc = "CHILLER",     .mArg = (void *)&p_chillerTempRead, .mFormat = MFORMAT_PARAM},
  {.desc = "BOIL SENSOR", .mArg = (void *)&p_boilTempRead,    .mFormat = MFORMAT_PARAM},
  {.desc = "BOIL SET",    .mArg = (void *)&p_boilTempSet,     .mFormat = MFORMAT_PARAM},
  {.desc = "BOIL DELTA",  .mArg = (void *)&p_boilTempDelta,   .mFormat = MFORMAT_PARAM},
  {.desc = "BOIL EN.",    .mArg = (void *)&p_boilOn,          .mFormat = MFORMAT_PARAM},
};
static struct menu_st gouglaszMenu = {
  .list = gouglaszMenuList,
  .msize = sizeof(gouglaszMenuList)/sizeof(struct menuItem_st),
  .pos = 0,
  .cursorPos = 0,
  .flags = 0,
  .paramcpy = 0,
};

// FUNCTIONS
// BUTTONS
static uint32_t bt_read(struct bt_st *btn);
static void bt_init(struct bt_st *btn, const uint8_t pin);
// ISR
static void timer1_overflow_ISR(void);
// MENU
static void menu_display(struct menu_st *menu);
static void menu_cursorDown(struct menu_st *menu);
static void menu_cursorUp(struct menu_st *menu);
static void menu_select(struct menu_st *menu);
static void menu_incParam(struct menu_st *menu);
static void menu_decParam(struct menu_st *menu);
// PARAM
static void paramGetValueStr(struct param_st *param, char *strout);

// BUTTONS
static void bt_init(struct bt_st *btn, const uint8_t pin) {
	btn->pin = pin;
	btn->holdstep = 0;
	btn->tick100ms = 0;
	pinMode(pin, INPUT_PULLUP);
}

static uint32_t bt_read(struct bt_st *btn) {
	uint32_t ret = 0;
	uint32_t holdtime_ms[4] = {400, 200, 100, 0};

	if (!digitalRead(btn->pin)) {
		if (btn->holdstep == 0) { // first press.
			ret = BT_EVENT | BT_PRESS;
			btn->holdstep = 1;
			btn->tick100ms = g_tick100ms;
		} else {
			ret = BT_HOLD;
			if (g_tick100ms -btn->tick100ms > holdtime_ms[btn->holdstep-1]/TIMERONE_TICK_MS) {
				ret |= BT_EVENT;
				btn->tick100ms = g_tick100ms;
				if (btn->holdstep < 4) { btn->holdstep++; }
			}
		}
	} else {
		btn->holdstep = 0;
	}
	return ret;
}

static struct tickTimer_st hltTimerOn;

// ISR
static void timer1_overflow_ISR(void) {
	g_tick100ms = ++g_tick50ms/2;
	
	//int heaterState;
	//// HLT heater
	//if (g_tick50ms%10 < p_hltHeaterPower.value/10) { heaterState = 1; } 
	//else { heaterState = 0; }
	//digitalWrite(HLT_HEATER_PIN, heaterState);
	//// Boil heater
	//if (g_tick50ms%10 < p_boilHeaterPower.value/10) { heaterState = 1; } 
	//else { heaterState = 0; }
	//digitalWrite(BOIL_HEATER_PIN, heaterState);
}

// MENU
static void menu_incParam(struct menu_st *menu) {
	const struct menuItem_st *menuItem = &menu->list[menu->cursorPos];
	struct param_st *param = (struct param_st *)menuItem->mArg;

	if (menuItem->mFormat == MFORMAT_PARAM) {		
		if ((menu->paramcpy +param->increment) > param->maxValue) { 
			menu->paramcpy = param->maxValue;
		} else {
			menu->paramcpy = (int16_t)(menu->paramcpy +param->increment);
		}
		if ((param->options & PARAM_OP_WCONFIRM) == 0) {
			param->value = menu->paramcpy; // write now
		}
	}
}

static void menu_decParam(struct menu_st *menu) {
	const struct menuItem_st *menuItem = &menu->list[menu->cursorPos];
	struct param_st *param = (struct param_st *)menuItem->mArg;

	if (menuItem->mFormat == MFORMAT_PARAM) {		
		if ((menu->paramcpy -param->increment) < param->minValue) { 
			menu->paramcpy = param->minValue;
		} else {
			menu->paramcpy = (int16_t)(menu->paramcpy -param->increment);
		}
		if ((param->options & PARAM_OP_WCONFIRM) == 0) {
			param->value = menu->paramcpy; // write now
		}
	}
}

static void menu_select(struct menu_st *menu) {
	const struct menuItem_st *menuItem = &menu->list[menu->cursorPos];
	struct param_st *param = (struct param_st *)menuItem->mArg;
	if ((menu->flags & MENU_FLAGS_EDITING) != 0) {
		menu->flags &= ~MENU_FLAGS_EDITING; // leave editing.
		param->value = menu->paramcpy;
	} else {
		if (menuItem->mFormat == MFORMAT_PARAM) {
			if (!PARAM_IS_RDONLY(param)) {
				menu->flags |= MENU_FLAGS_EDITING; // enter editing
				menu->paramcpy = param->value;
			}
		}
	}
}

static void menu_display(struct menu_st *menu) {
	uint8_t line;
	uint8_t listItem;
	char lineStr[21];
	const struct menuItem_st *menuItem;
	
	// display menu screen
	for (line = 0; line < 4; line++) {
		memset(lineStr, ' ', sizeof(lineStr) -1);
		lineStr[sizeof(lineStr) -1] = '\0';
		listItem = menu->pos +line;
		menuItem = &menu->list[listItem];
		if (listItem == menu->cursorPos) {
			// display cursor position character.
			lineStr[0] = '>';
		}
		if (listItem < menu->msize) {
			// display item description & value
			const char *descstr = menuItem->desc;
			strncpy(&lineStr[1], descstr, strlen(descstr));
			if (menuItem->mArg != NULL) {
				char valuestr[12];
				memset(valuestr, 0, sizeof(valuestr));
				paramGetValueStr((struct param_st *)menuItem->mArg, valuestr);
				// blinks value when editing
				if ((menu->flags & MENU_FLAGS_EDITING) != 0 && listItem == menu->cursorPos && (g_tick100ms%8) < 3) {
					
				} else {
					strncpy(&lineStr[20 -strlen(valuestr)], valuestr, strlen(valuestr));
				}
			}
		} else {
			break; // no more items to display.
		}
		lcd.setCursor(0, line);
		lcd.print(lineStr);
	}
}

static void menu_cursorDown(struct menu_st *menu) {
	if (menu->cursorPos < menu->msize -1) {
		if (menu->cursorPos -menu->pos +1 >= 4) {
			menu->pos++;
			menu->cursorPos++;
		} else {
			menu->cursorPos++;
		}
	}
}

static void menu_cursorUp(struct menu_st *menu) {
	if (menu->cursorPos > 0) {
		if (menu->cursorPos == menu->pos) {
			menu->pos--;
			menu->cursorPos--;
		} else {
			menu->cursorPos--;
		}
	}
}

// PARAM
static void paramGetValueStr(struct param_st *param, char *strout) {
	char valuestr[10];
	int power = 1;
	int integer = 0;
	int decimal = 0;
	int paramValue;

	if ((param->options & PARAM_OP_DEC1) != 0) { power = 10; }
	else if ((param->options & PARAM_OP_DEC2) != 0) { power = 100; }
	else if ((param->options & PARAM_OP_DEC3) != 0) { power = 1000; }
	else { power = 1; }

	paramValue = (int)((int16_t)param->value);
	integer = paramValue/power;
	if (power > 1) { decimal = paramValue -(paramValue/power)*power; }

	if (paramValue < 0) { strcat(strout, "-"); }
	// print interger part.
	integer = integer < 0 ? -integer : integer; // abs.
	sprintf(valuestr, "%i", integer);
	strcat(strout, valuestr);
	if (power > 1) {  // add decimal value
		decimal = decimal < 0 ? -decimal : decimal; // abs.
		sprintf(valuestr, ".%i", decimal);
		strcat(strout, valuestr); 
	}
}

static void menu_handleEvent(struct menu_st *menu, const enum menuEvent_en event) {
	if ((menu->flags & MENU_FLAGS_EDITING) != 0) {
		if (event == MNEV_ENTER) { 
			menu_select(menu);
		} else if (event == MNEV_UP) { 
			menu_incParam(menu);
		} else if (event == MNEV_DOWN) { 
			menu_decParam(menu);
		}
	} else {
		if (event == MNEV_ENTER) { 
			menu_select(menu);
		} else if (event == MNEV_UP) { 
			menu_cursorUp(menu);
		} else if (event == MNEV_DOWN) { 
			menu_cursorDown(menu);
		}
	}
}

static void appHLT_timerToTurnOn(void);
static void appHLT_tempControlHyst(void);

// SETUP - runs once.
void setup(void) {
  memset(&hltTimerOn, 0, sizeof(struct tickTimer_st));

  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);
  // Print the splash screen msg.
  lcd.setCursor(0, 0);
  lcd.print("********************");
  lcd.setCursor(0, 1);
  lcd.print("****  UN-NAMED  ****");
  lcd.setCursor(0, 2);
  lcd.print("****  BREW v3.0 ****");
  lcd.setCursor(0, 3);
  lcd.print("********************");
  delay(2000);

  // IO pin configuration
  
  pinMode(HLT_HEATER_PIN, OUTPUT);
  pinMode(BOIL_HEATER_PIN, OUTPUT);
  
  digitalWrite(HLT_HEATER_PIN, 0);
  digitalWrite(BOIL_HEATER_PIN, 0);
  
  bt_init(&btup, BT_UP_PIN);
  bt_init(&btdown, BT_DOWN_PIN);
  bt_init(&btenter, BT_ENTER_PIN);
  bt_init(&btcancel, BT_CANCEL_PIN);

  // Timer one interrupt
  Timer1.initialize(TIMERONE_TICK_US); // initialize timer1.
  Timer1.attachInterrupt(timer1_overflow_ISR);  // attaches callback() as a timer overflow interrupt

  // sesor temperatures
  sensors.begin();
  sensors.setResolution(tempSensorAddr_HLT, TEMPERATURE_PRECISION);
  sensors.setResolution(tempSensorAddr_BOIL, TEMPERATURE_PRECISION);
  sensors.setResolution(tempSensorAddr_CHILLER, TEMPERATURE_PRECISION);
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  
  Serial.begin(9600);
}

// LOOP - runs after setup, in loop, duh!
void loop(void) {
  enum menuEvent_en mevent = MNEV_NONE;
  // BUTTONS
  uint32_t btret;
  btret = bt_read(&btup);  //UP
  if (btret & BT_EVENT) { mevent = MNEV_UP; }
  btret = bt_read(&btdown); //DOWN
  if (btret & BT_EVENT) { mevent = MNEV_DOWN; }
  btret = bt_read(&btenter); //ENTER
  if (btret & BT_EVENT) { mevent = MNEV_ENTER; }
  btret = bt_read(&btcancel); //CANCEL
  if (btret & BT_EVENT) { mevent = MNEV_CANCEL; }
  // MENU
  menu_handleEvent(&gouglaszMenu, mevent);
  menu_display(&gouglaszMenu);

  // Update sensor temperatures
  float tempf;
  
  tempf = sensors.getTempC(tempSensorAddr_HLT);
  p_hltLineTempRead.value = (int16_t)(tempf*10);
  
  tempf = sensors.getTempC(tempSensorAddr_BOIL);
  p_boilTempRead.value = (int16_t)(tempf*10);
  
  tempf = sensors.getTempC(tempSensorAddr_CHILLER);
  p_chillerTempRead.value = (int16_t)(tempf*10);
  
  sensors.requestTemperatures(); // Send the command to get temperatures

  appHLT_timerToTurnOn();
  appHLT_tempControlHyst();
  appBOIL_tempControlHyst();
  
  delay(LOOP_DELAY_MS);
}

#define TIMERAPP_STOPPED 0
#define TIMERAPP_RUNNING 1

static void appHLT_timerToTurnOn(void) {
  static int16_t time2Cook = 0;
  static uint32_t timeSum = 0;
  if (time2Cook != p_hltTime2Cook.value) {
    // new timer configuration.
    time2Cook = p_hltTime2Cook.value;
    p_hltCookTimeLeft.value = time2Cook;
    timeSum = millis();
  }
  if (p_hltCookTimeLeft.value > 0) {
    if ((uint32_t)(millis() - timeSum) > 60000) {
      timeSum = millis();
      if (p_hltCookTimeLeft.value == 1) {
        // turn HLT on
        p_hltOn.value = 1;
        p_boilOn.value = 1;
        p_hltTime2Cook.value = 0;
      }
      p_hltCookTimeLeft.value--;
    }
  }
}

static void appHLT_tempControlHyst(void) {
  if (p_hltOn.value == 1) {
    if (p_hltLineTempRead.value > (p_hltTempSet.value + p_hltTempDelta.value)) {
      digitalWrite(HLT_HEATER_PIN, 0);
      Serial.println("Heater OFF");
    } else if (p_hltLineTempRead.value < (p_hltTempSet.value)) {
      digitalWrite(HLT_HEATER_PIN, 1);
      Serial.println("Heater ON");
    }
  } else {
    digitalWrite(HLT_HEATER_PIN, 0);
  }
}

static void appBOIL_tempControlHyst(void) {
  if (p_boilOn.value == 1) {
    if (p_boilTempRead.value > (p_boilTempSet.value + p_boilTempDelta.value)) {
      digitalWrite(BOIL_HEATER_PIN, 0);
      Serial.println("Heater OFF");
    } else if (p_boilTempRead.value < (p_boilTempSet.value)) {
      digitalWrite(BOIL_HEATER_PIN, 1);
      Serial.println("Heater ON");
    }
  } else {
    digitalWrite(BOIL_HEATER_PIN, 0);
  }
}

