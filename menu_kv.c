#include "menu_kv.h"
#include <string.h>
#include <stdint.h>

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


#define TIMERONE_TICK_US 50000
#define TIMERONE_TICK_MS (TIMERONE_TICK_US / 1000)
#define LOOP_DELAY_MS 50

// MENU
#define MENU_FLAGS_EDITING 0x40


static struct bt_st btup, btdown, btenter, btcancel;
// global tick
static volatile uint32_t g_tick100ms = 0;
static volatile uint32_t g_tick50ms = 0;


// FUNCTIONS
// BUTTONS
static uint32_t bt_read(struct bt_st *btn);
static void bt_init(struct bt_st *btn, const uint8_t pin);
// MENU
static void menu_cursorDown(struct menu_st *menu);
static void menu_cursorUp(struct menu_st *menu);
static void menu_select(struct menu_st *menu);
static void menu_incParam(struct menu_st *menu);
static void menu_decParam(struct menu_st *menu);
// PARAM
static void paramGetValueStr(struct kv_i32_st *param, char *strout);

// BUTTONS
static void bt_init(struct bt_st *btn, const uint8_t pin) {
	btn->holdstep = 0;
	btn->tick100ms = 0;
	///pinMode(pin, INPUT_PULLUP);
}

static uint32_t bt_read(struct bt_st *btn) {
	uint32_t ret = 0;
	uint32_t holdtime_ms[4] = {400, 200, 100, 0};

	//if (!digitalRead(btn->pin)) {
	if (1) {
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

// MENU
static void menu_incParam(struct menu_st *menu) {
	const struct menuItem_st *menuItem = &menu->list[menu->cursorPos];
	struct kv_i32_st *param = (struct kv_i32_st *)menuItem->mArg;

	if (menuItem->mFormat == MFORMAT_PARAM) {		
		if ((menu->paramcpy +param->inc) > param->max) {
			menu->paramcpy = param->max;
		} else {
			menu->paramcpy = (int16_t)(menu->paramcpy +param->inc);
		}
		if ((param->op & KV_OP_WCONFIRM) == 0) {
			param->val = menu->paramcpy; // write now
		}
	}
}

static void menu_decParam(struct menu_st *menu) {
	const struct menuItem_st *menuItem = &menu->list[menu->cursorPos];
	struct kv_i32_st *param = (struct kv_i32_st *)menuItem->mArg;

	if (menuItem->mFormat == MFORMAT_PARAM) {		
		if ((menu->paramcpy -param->inc) < param->min) {
			menu->paramcpy = param->min;
		} else {
			menu->paramcpy = (int16_t)(menu->paramcpy -param->inc);
		}
		if ((param->op & KV_OP_WCONFIRM) == 0) {
			param->val = menu->paramcpy; // write now
		}
	}
}

static void menu_select(struct menu_st *menu) {
	const struct menuItem_st *menuItem = &menu->list[menu->cursorPos];
	struct kv_i32_st *param = (struct kv_i32_st *)menuItem->mArg;
	if ((menu->flags & MENU_FLAGS_EDITING) != 0) {
		menu->flags &= ~MENU_FLAGS_EDITING; // leave editing.
		param->val = menu->paramcpy;
	} else {
		if (menuItem->mFormat == MFORMAT_PARAM) {
			if (!KV_IS_RDONLY(param)) {
				menu->flags |= MENU_FLAGS_EDITING; // enter editing
				menu->paramcpy = param->val;
			}
		}
	}
}

void menu_display(struct menu_st *menu, const int line, char *line_buff, const int size) {
	uint8_t listItem;
	const struct menuItem_st *menuItem;
	
    memset(line_buff, ' ', size -1);
    line_buff[size -1] = '\0';
    listItem = menu->pos +line;
    menuItem = &menu->list[listItem];
    if (listItem < menu->msize) {
        if (listItem == menu->cursorPos) {
            // display cursor position character.
            line_buff[0] = '>';
        }
        // display item description & value
        const char *descstr = menuItem->desc;
        strncpy(&line_buff[1], descstr, strlen(descstr));
        if (menuItem->mArg != NULL) {
            char valuestr[12];
            memset(valuestr, 0, sizeof(valuestr));
            paramGetValueStr((struct kv_i32_st *)menuItem->mArg, valuestr);
            // blinks value when editing
            if ((menu->flags & MENU_FLAGS_EDITING) != 0 && listItem == menu->cursorPos && (g_tick100ms%8) < 3) {

            } else {
                strncpy(&line_buff[(size - 1) -strlen(valuestr)], valuestr, strlen(valuestr));
            }
        }
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
static void paramGetValueStr(struct kv_i32_st *param, char *strout) {
	char valuestr[10];
	int power = 1;
	int integer = 0;
	int decimal = 0;
	int paramValue;

	if ((param->op & KV_OP_DEC1) != 0) { power = 10; }
	else if ((param->op & KV_OP_DEC2) != 0) { power = 100; }
	else if ((param->op & KV_OP_DEC3) != 0) { power = 1000; }
	else { power = 1; }

	paramValue = (int)((int16_t)param->val);
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

void menu_handleEvent(struct menu_st *menu, const enum menuEvent_en event) {
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
/*
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
*/
