#ifndef KV_H_
#define KV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct kv_meta_struct {
	uint32_t op;
	uint32_t max_val;
	uint32_t min_val;
	uint32_t inc_val;
	uint32_t init_val;
	void *ptr;
} kv_meta_t;

#define KV_I32_OP_ENUMERATOR(keyname_, init_val_, min_val_, max_val_, inc_val_, options_) keyname_,
#define KV_ENUM_OP_ENUMERATOR(keyname_, init_val_, max_val_, ...) keyname_,

#include "kv_conf.h"

typedef enum kv_key_enum {

    KV_I32_LIST_START,
    KV_I32_LIST(KV_I32_OP_ENUMERATOR)
    KV_I32_LIST_END,

    KV_ENUM_LIST_START,
    KV_ENUM_LIST(KV_ENUM_OP_ENUMERATOR)
    KV_ENUM_LIST_END,

    KV_ENUM_SIZE,
} kv_key_t;

#define KV_I32_NITEMS (KV_I32_LIST_END - KV_I32_LIST_START -1)
#define KV_ENUM_NITEMS (KV_ENUM_LIST_END - KV_ENUM_LIST_START -1)

void kv_print();

#ifdef __cplusplus
}
#endif

#endif /* KV_H_ */
