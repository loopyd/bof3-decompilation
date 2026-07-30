#include "internal.h"

/* @source 0x801DBB20 */
/* @behavior Returns the first of three local work records whose byte +0x79 matches the selector, or NULL. */
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
