// SPDX-License-Identifier: MIT

#include "menu_kv.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define MENU_FLAGS_EDITING 0x40

static void menu_cursorDown(struct menu_st *menu);
static void menu_cursorUp(struct menu_st *menu);
static void menu_select(struct menu_st *menu);
static void menu_incParam(struct menu_st *menu);
static void menu_decParam(struct menu_st *menu);
static void paramGetValueStr(struct kv_i32_st *param, char *strout);


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
		menu->flags &= ~MENU_FLAGS_EDITING; // leave editing
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
            char valuestr[16];
            char *value_ptr = valuestr;
            memset(valuestr, 0, sizeof(valuestr));

            // blinks value when editing
            if ((menu->flags & MENU_FLAGS_EDITING) != 0 && listItem == menu->cursorPos) {
                valuestr[0] = '[';
                paramGetValueStr((struct kv_i32_st *)menuItem->mArg, &valuestr[1]);
                int len = strlen(valuestr);
                valuestr[len] = ']';
            } else {
                paramGetValueStr((struct kv_i32_st *)menuItem->mArg, &valuestr[0]);
            }
            strncpy(&line_buff[(size - 1) -strlen(valuestr)], valuestr, strlen(valuestr));
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

void menu_cancel(struct menu_st *menu) {
	const struct menuItem_st *menuItem = &menu->list[menu->cursorPos];
	if ((menu->flags & MENU_FLAGS_EDITING) != 0) {
		menu->flags &= ~MENU_FLAGS_EDITING; // leave editing
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
