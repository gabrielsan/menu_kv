// SPDX-License-Identifier: MIT

#ifndef MENU_KV_
#define MENU_KV_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TTFLG_ON 0x1
#define TTFLG_UNDERFLOWED 0x2

#define KV_OP_WCONFIRM 0x08
#define KV_OP_DEC0 0x10
#define KV_OP_DEC1 0x20
#define KV_OP_DEC2 0x40
#define KV_OP_DEC3 0x80
#define KV_OP_RDONLY 0x800

#define KV_IS_RDONLY(param) ((param->op & KV_OP_RDONLY) != 0)

#define KV_NAME(name_) (kv_##name_)

#define M_ENTRY_KV(desc_, name_) {.desc = desc_, .mArg = (void *)&kv_##name_, .mFormat = MFORMAT_PARAM}


#define KV_INT32(name_, init_val_, min_val_, max_val_, inc_val_, options_) \
static struct kv_i32_st kv_##name_ = {                                     \
	.op = options_,                                                        \
	.val = (int32_t)init_val_,                                             \
	.max = max_val_,                                                       \
	.min = min_val_,                                                       \
	.inc = inc_val_,                                                       \
};

enum menuEvent_en {
	MNEV_UP,
	MNEV_DOWN,
	MNEV_ENTER,
	MNEV_CANCEL,
	MNEV_NONE,
};


struct bt_st {
	uint32_t tick100ms;
	uint8_t holdstep;
	uint8_t reserved[2]; // align(4)
};

struct kv_i32_st {
	const uint32_t op;
	int32_t val;
	const int32_t max;
	const int32_t min;
	const int32_t inc;
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

void menu_cancel(struct menu_st *menu);
void menu_display(struct menu_st *menu, const int line, char *line_buff, const int size);
void menu_handleEvent(struct menu_st *menu, const enum menuEvent_en event);

#ifdef __cplusplus
}
#endif

#endif /* MENU_KV_ */
