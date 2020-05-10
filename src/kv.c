#include "kv.h"
#include <stdint.h>
#include <stdio.h>

#ifndef KV_META_ATTR
#define KV_META_ATTR aligned
#endif /* KV_META_ATTR */

#define KV_I32_VAL(keyname_, init_val_, min_val_, max_val_, inc_val_, options_) init_val_,

#define KV_I32_META_OP(keyname_, init_val_, min_val_, max_val_, inc_val_, options_) { \
    .init_val = (uint32_t)init_val_, \
    .inc_val = (uint32_t)inc_val_, \
    .max_val = (uint32_t)max_val_, \
    .min_val = (uint32_t)min_val_, \
    .op = options_, \
},

__attribute__ ((KV_META_ATTR))
static const kv_meta_t kv_i32_meta[KV_I32_ITEMS] = {KV_I32_LIST(KV_I32_META_OP)};

static int32_t kv_i32_list[KV_I32_ITEMS];

void kv_print(void) {

    for(int k = KV_ENUM_I32_START; k < KV_I32_ITEMS; k++) {
        printf("#%d = %d\n", k, kv_i32_list[k]);
        
        printf("#%d [%d|%d|%d|%d|%d]\n", k, kv_i32_meta[k].init_val, kv_i32_meta[k].inc_val, kv_i32_meta[k].max_val, kv_i32_meta[k].min_val, kv_i32_meta[k].op);
    }
}

