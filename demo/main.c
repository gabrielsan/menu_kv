#include <stdlib.h>
#include <stdio.h>

#include "menu_kv.h"
#include "kv.h"

#define assert(x)                   \
    if (!(x)) {                     \
            printf("assertion\n");  \
            exit(1);                \
    }

int main(int argc, char **argv) {
    printf("Hello World!\n");

    // print all enums

    for (keyID key = KV_ENUM_LIST_START +1; key < KV_ENUM_LIST_END; key++) {
        uint8_t max_val;
        int32_t ret;
        ret = kv_enum_get_meta(key, &max_val);
        assert(ret == 0);
        for (uint8_t val = 0; val <= max_val; val++) {
            char const *enum_string = NULL;
            ret = kv_enum_read_string(key, val, &enum_string);
            if (ret == 0) {
                printf("Enum %d|%d:%s\n", key, val, enum_string);
            } else {
                printf("Enum %d|%d:error\n", key, val);
                break;;
            }
        }
    }

    exit(0);
}
