#include "internal.h"

Battle03LocalWork *func_801DBB20(u8 arg0)
{
    u8 i;

    for (i = 0; i < 3; i++) {
        if (D_80145E90[i].unk_79 == arg0) {
            return &D_80145E90[i];
        }
    }

    return NULL;
}
