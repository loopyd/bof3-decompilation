#include "bof3/battle/battle03_internal.h"

/* @source 0x801DBB20
 * @behavior Scans the three 0x140-byte records at D_80145E90 in index order
 * and returns the first whose byte +0x79 equals arg0. Returns NULL when none
 * matches.
 * @status exact
 * @match 100.00
 * @residual none
 */
Battle03LocalWork *findLocalWorkByKind(u8 arg0)
{
    u8 i;

    for (i = 0; i < 3; i++) {
        if (D_80145E90[i].unk_79 == arg0) {
            return &D_80145E90[i];
        }
    }

    return NULL;
}
