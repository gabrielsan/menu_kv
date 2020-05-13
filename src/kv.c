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

void kv_init(void) {

}

void kv_print(void) {

    for(int k = KV_I32_LIST_START; k < KV_I32_NITEMS; k++) {
        printf("#%d = %d\n", k, kv_i32_value[k]);
        
        printf("#%d [%d|%d|%d|%d]\n", k, kv_i32_meta[k].std_val, kv_i32_meta[k].max_val, kv_i32_meta[k].min_val, kv_i32_meta[k].op);
    }
    for(int k = 0; k < KV_ENUM_NITEMS; k++) {
        const char **print_str = kv_enum_meta[k].ptr;
        if (print_str == NULL) {
            printf("#%d NULL\n", k);
        } else {
            for(uint32_t j = 0; j < (2 + kv_enum_meta[k].max_val); j++) {
                if (print_str[j] != NULL) {
                    printf("#%d|%d = %s\n", k, j, print_str[j]);
                } else {
                    printf("#%d NULL\n", k);
                    break;
                }
            }
        }
    }
}

