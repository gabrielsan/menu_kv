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
	uint32_t std_val;
	void *ptr;
} kv_meta_t;

#define KV_I32_OP_ENUMERATOR(keyname_, std_val_, min_val_, max_val_, options_) keyname_,
#define KV_ENUM_OP_ENUMERATOR(keyname_, std_val_, max_val_, ...) keyname_,

#include "kv_conf.h"

typedef enum {

    KV_I32_LIST_START,
    KV_I32_LIST(KV_I32_OP_ENUMERATOR)
    KV_I32_LIST_END,

    KV_ENUM_LIST_START,
    KV_ENUM_LIST(KV_ENUM_OP_ENUMERATOR)
    KV_ENUM_LIST_END,

    KV_ENUM_SIZE,
} keyID;

#define KV_I32_NITEMS (KV_I32_LIST_END - KV_I32_LIST_START -1)
#define KV_ENUM_NITEMS (KV_ENUM_LIST_END - KV_ENUM_LIST_START -1)

int32_t kv_init(void);
int32_t kv_enum_read_string(const keyID key, const uint8_t val, char const **str_out);
int32_t kv_enum_get_meta(const keyID key, uint8_t *max_val);
int32_t kv_enum_read(const keyID key, uint8_t *val);
int32_t kv_enum_write(const keyID key, const uint8_t val);
int32_t kv_i32_read(const keyID key, int32_t *val);
int32_t kv_i32_write(const keyID key, const int32_t val);

#ifdef __cplusplus
}
#endif

#endif /* KV_H_ */
