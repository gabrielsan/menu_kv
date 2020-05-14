#include <stdint.h>
#include <stdio.h>
#include "kv.h"

#ifndef KV_META_ATTR
#define KV_META_ATTR aligned
#endif /* KV_META_ATTR */

#ifndef KV_ASSERT
#define KV_ASSERT(x)                                                      \
    if (!(x)) { for (;;); }
#endif /* KV_ASSERT */

#define KV_I32_OP_META(keyname_, std_val_, min_val_, max_val_, options_)  \
{                                                                         \
    .std_val = (uint32_t)std_val_,                                        \
    .max_val = (uint32_t)max_val_,                                        \
    .min_val = (uint32_t)min_val_,                                        \
    .op = options_,                                                       \
    .ptr = NULL,                                                          \
},

#define KV_ENUM_OP_META(keyname_, std_val_, max_val_, ...)                \
{                                                                         \
    .std_val = (uint32_t)std_val_,                                        \
    .max_val = (uint32_t)(max_val_ - 1),                                  \
    .min_val = 0,                                                         \
    .op = 0,                                                              \
    .ptr = kv_enum_strlist_ ## keyname_,                                  \
},

#define KV_ENUM_OP_STR_LIST(keyname_, std_val_, max_val_, ...)            \
static const char *kv_enum_strlist_ ## keyname_[] = {                     \
    __VA_ARGS__ __VA_OPT__(,) NULL                                        \
};

typedef struct {
	uint32_t op;
	uint32_t max_val;
	uint32_t min_val;
	uint32_t std_val;
	void *ptr;
} kvMeta;

__attribute__ ((KV_META_ATTR))
KV_ENUM_LIST(KV_ENUM_OP_STR_LIST)

__attribute__ ((KV_META_ATTR))
static const kvMeta kv_i32_meta[KV_I32_NITEMS] = {KV_I32_LIST(KV_I32_OP_META)};

__attribute__ ((KV_META_ATTR))
static const kvMeta kv_enum_meta[KV_ENUM_NITEMS] = {KV_ENUM_LIST(KV_ENUM_OP_META)};

static int32_t kv_i32_value[KV_I32_NITEMS];

static uint8_t kv_enum_value[KV_ENUM_NITEMS];

int32_t kv_init(void) {
    for (uint32_t k = 0; k < KV_ENUM_NITEMS; k++) {
        // set all enum values to standard value
        uint8_t enum_std = (uint8_t)kv_enum_meta[k].std_val;
        uint8_t enum_max = (uint8_t)kv_enum_meta[k].max_val;
        KV_ASSERT(enum_std <= enum_max);
        kv_enum_value[k] = enum_std;
    }

    for (uint32_t k = 0; k < KV_I32_NITEMS; k++) {
        // set all i32 values to standard value
        int32_t i32_std = (int32_t)kv_i32_meta[k].std_val;
        int32_t i32_max = (int32_t)kv_i32_meta[k].max_val;
        int32_t i32_min = (int32_t)kv_i32_meta[k].min_val;
        KV_ASSERT(i32_std <= i32_max);
        KV_ASSERT(i32_std >= i32_min);
        kv_i32_value[k] = i32_std;
    }

    return 0;
}

char const *kv_enum_get_string(const keyID key, const uint8_t val) {
    if (key <=  KV_ENUM_LIST_START || key >= KV_ENUM_LIST_END) return NULL;

    keyID key_rel = key - (KV_ENUM_LIST_START + 1);

    uint8_t enum_max = (uint8_t)kv_enum_meta[key_rel].max_val;
    if (val > enum_max) return NULL; // out of bounds

    const char **enum_str_list = kv_enum_meta[key_rel].ptr;
    for (uint8_t j = 0; j <= val; j++) {
        if (enum_str_list[j] == NULL) return NULL; // string not defined
    }

    return enum_str_list[val];
}

uint8_t kv_enum_get_nitems(const keyID key) {
    if (key <=  KV_ENUM_LIST_START || key >= KV_ENUM_LIST_END) return 0;

    keyID key_rel = key - (KV_ENUM_LIST_START + 1);

    return (uint8_t)(kv_enum_meta[key_rel].max_val + 1);
}

int32_t kv_enum_read_std(const keyID key, uint8_t *val) {
    if (key <=  KV_ENUM_LIST_START || key >= KV_ENUM_LIST_END) return -1;

    keyID key_rel = key - (KV_ENUM_LIST_START + 1);

    *val = kv_enum_meta[key_rel].std_val;
    return 0;
}

int32_t kv_enum_read(const keyID key, uint8_t *val) {
    if (key <=  KV_ENUM_LIST_START || key >= KV_ENUM_LIST_END) return -1;

    keyID key_rel = key - (KV_ENUM_LIST_START + 1);

    *val = kv_enum_value[key_rel];
    return 0;
}

int32_t kv_enum_write(const keyID key, const uint8_t val) {
    if (key <=  KV_ENUM_LIST_START || key >= KV_ENUM_LIST_END) return -1;

    keyID key_rel = key - (KV_ENUM_LIST_START + 1);

    uint8_t enum_val = val;
    uint8_t enum_max = (uint8_t)kv_enum_meta[key_rel].max_val;

    if (enum_val > enum_max) enum_val = enum_max;

    kv_enum_value[key_rel] = enum_val;
    return 0;
}

int32_t kv_i32_read_std(const keyID key, int32_t *val) {
    if (key <= KV_I32_LIST_START || key >= KV_I32_LIST_END) return -1;

    keyID key_rel = key - (KV_I32_LIST_START + 1);

    *val = kv_i32_meta[key_rel].std_val;
    return 0;
}

int32_t kv_i32_read(const keyID key, int32_t *val) {
    if (key <= KV_I32_LIST_START || key >= KV_I32_LIST_END) return -1;

    keyID key_rel = key - (KV_I32_LIST_START + 1);

    *val = kv_i32_value[key_rel];
    return 0;
}

int32_t kv_i32_write(const keyID key, const int32_t val) {
    if (key <= KV_I32_LIST_START || key >= KV_I32_LIST_END) return -1;

    keyID key_rel = key - (KV_I32_LIST_START + 1);

    int32_t i32_val = val;
    int32_t i32_max = (int32_t)kv_i32_meta[key_rel].max_val;
    int32_t i32_min = (int32_t)kv_i32_meta[key_rel].min_val;

    if (i32_val < i32_min) i32_val = i32_min;
    if (i32_val > i32_max) i32_val = i32_max;

    kv_i32_value[key_rel] = i32_val;
    return 0;
}

