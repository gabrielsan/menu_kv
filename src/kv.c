#include "kv.h"
#include <stdint.h>
#include <stdio.h>

#ifndef KV_META_ATTR
#define KV_META_ATTR aligned
#endif /* KV_META_ATTR */

#define KV_I32_META_OP(keyname_, std_val_, min_val_, max_val_, options_)            \
{                                                                                   \
    .std_val = (uint32_t)std_val_,                                                  \
    .max_val = (uint32_t)max_val_,                                                  \
    .min_val = (uint32_t)min_val_,                                                  \
    .op = options_,                                                                 \
    .ptr = NULL,                                                                    \
},

#define KV_ENUM_OP_META(keyname_, std_val_, max_val_, ...)  \
{                                                           \
    .std_val = (uint32_t)std_val_,                          \
    .max_val = (uint32_t)(max_val_ - 1),                    \
    .min_val = 0,                                           \
    .op = 0,                                                \
    .ptr = kv_enum_strlist_ ## keyname_,                    \
},

#define KV_ENUM_OP_STR_LIST(keyname_, std_val_, max_val_, ...)                            \
    static const char *kv_enum_strlist_ ## keyname_[] = { __VA_ARGS__ __VA_OPT__(,) NULL}; \

__attribute__ ((KV_META_ATTR))
KV_ENUM_LIST(KV_ENUM_OP_STR_LIST)

__attribute__ ((KV_META_ATTR))
static const kv_meta_t kv_i32_meta[KV_I32_NITEMS] = {KV_I32_LIST(KV_I32_META_OP)};

__attribute__ ((KV_META_ATTR))
static const kv_meta_t kv_enum_meta[KV_ENUM_NITEMS] = {KV_ENUM_LIST(KV_ENUM_OP_META)};

static int32_t kv_i32_value[KV_I32_NITEMS];
static uint8_t kv_enum_value[KV_ENUM_NITEMS];

int32_t kv_init(void) {
    // set all enum values to standard value
    for (uint32_t k = 0; k < KV_ENUM_NITEMS; k++) {
        kv_enum_value[k] = (uint8_t)kv_enum_meta[k].std_val;
    }

    // set all i32 values to standard value
    for (uint32_t k = 0; k < KV_I32_NITEMS; k++) {
        kv_i32_value[k] = (int32_t)kv_i32_meta[k].std_val;
    }

    return 0;
}

int32_t kv_enum_read_string(const keyID key, const uint8_t val, char const **str_out) {
    int32_t ret = -1;
    if (key >  KV_ENUM_LIST_START && key < KV_ENUM_LIST_END) {
        keyID key_rel = key - (KV_ENUM_LIST_START + 1);

        uint8_t enum_val = val;
        uint8_t enum_max = (uint8_t)kv_enum_meta[key_rel].max_val;
        if (val > enum_max) return -1; // out of bounds

        const char **enum_str_list = kv_enum_meta[key_rel].ptr;
        for (uint8_t j = 0; j <= val; j++) {
            if (enum_str_list[j] == NULL) return -1; // string not defined
        }
        
        *str_out = enum_str_list[val];
        ret = 0;
    }
    return ret;
}


int32_t kv_enum_get_meta(const keyID key, uint8_t *max_val) {
    int32_t ret = -1;
    if (key >  KV_ENUM_LIST_START && key < KV_ENUM_LIST_END) {
        keyID key_rel = key - (KV_ENUM_LIST_START + 1);

        *max_val = kv_enum_meta[key_rel].max_val;
        ret = 0;
    }
    return ret;
}

int32_t kv_enum_read(const keyID key, uint8_t *val) {
    int32_t ret = -1;
    if (key >  KV_ENUM_LIST_START && key < KV_ENUM_LIST_END) {
        keyID key_rel = key - (KV_ENUM_LIST_START + 1);

        *val = kv_enum_value[key_rel];
        ret = 0;
    }
    return ret;
}

int32_t kv_enum_write(const keyID key, const uint8_t val) {
    int32_t ret = -1;
    if (key >  KV_ENUM_LIST_START && key < KV_ENUM_LIST_END) {

        keyID key_rel = key - (KV_ENUM_LIST_START + 1);

        uint8_t enum_val = val;
        uint8_t enum_max = (uint8_t)kv_enum_meta[key_rel].max_val;

        if (enum_val > enum_max) enum_val = enum_max;

        kv_enum_value[key_rel] = enum_val;
        ret = 0;
    }
    return ret;
}


int32_t kv_i32_read(const keyID key, int32_t *val) {
    int32_t ret = -1;
    if (key >  KV_I32_LIST_START && key < KV_I32_LIST_END) {

        keyID key_rel = key - (KV_I32_LIST_START + 1);

        *val = kv_i32_value[key_rel];
        ret = 0;
    }
    return ret;
}

int32_t kv_i32_write(const keyID key, const int32_t val) {
    int32_t ret = -1;
    if (key >  KV_I32_LIST_START && key < KV_I32_LIST_END) {

        keyID key_rel = key - (KV_I32_LIST_START + 1);

        int32_t i32_val = val;
        int32_t i32_max = (int32_t)kv_i32_meta[key_rel].max_val;
        int32_t i32_min = (int32_t)kv_i32_meta[key_rel].min_val;

        if (i32_val < i32_min) i32_val = i32_min;
        if (i32_val > i32_max) i32_val = i32_max;

        kv_i32_value[key_rel] = i32_val;
        ret = 0;
    }
    return ret;
}

