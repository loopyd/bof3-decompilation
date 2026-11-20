#include "bof3/world/area03004_internal.h"

/* @behavior walks the 0x1E shared 0x98-byte work records at 0x80146888; for
 * every active record it publishes that record as the scratchpad cursor at
 * 0x1F800044 and runs the per-record scroll updater updateWorkScrollWordsWorld00Area03004_801E084C.
 * @source 0x801E09B4
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateActiveWorkRecordScrollWordsWorld00Area03004(void) {
    Area030WorkRecord *record;
    s32 recordIndex;

    for (recordIndex = 0; recordIndex < 0x1E; recordIndex++) {
        if (D_80146888[recordIndex].flags_00 != 0) {
            record = &D_80146888[recordIndex];
            D_1F800044 = (u8 *)record;
            updateWorkScrollWordsWorld00Area03004_801E084C();
        }
    }
}
