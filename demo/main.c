#include <stdlib.h>
#include <stdio.h>
#include "kv.h"

#define assert(x)                   \
    if (!(x)) {                     \
            printf("assertion\n");  \
            exit(1);                \
    }

int main(int argc, char **argv) {
    printf("Hello World!\n");

    int32_t ret;

    ret = kv_init();
    assert(ret == 0);

    // print all enums
    for (keyID key = KV_ENUM_LIST_START +1; key < KV_ENUM_LIST_END; key++) {
        uint8_t nitems = kv_enum_get_nitems(key);
        for (uint8_t j = 0; j < nitems; j++) {
            char const *enum_string = kv_enum_get_string(key, j);
            if (enum_string != NULL) {
                printf("Enum %d|%d:%s\n", key, j, enum_string);
            } else {
                printf("Enum %d|%d:error\n", key, j);
                break;
            }
        }
    }

    exit(0);
}
