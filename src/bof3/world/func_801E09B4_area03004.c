#include "bof3/world/area03004_internal.h"

/* @behavior walks the 0x1E shared 0x98-byte work records at 0x80146888; for
 * every active record it publishes that record as the scratchpad cursor at
 * 0x1F800044 and runs the per-record scroll updater func_801E084C.
 * @source 0x801E09B4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E09B4(void) {
    Area030WorkRecord *record;
    s32 i;

    for (i = 0; i < 0x1E; i++) {
        if (D_80146888[i].flags_00 != 0) {
            record = &D_80146888[i];
            D_1F800044 = (u8 *)record;
            func_801E084C();
        }
    }
}
