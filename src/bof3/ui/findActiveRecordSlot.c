#include "bof3/bof3.h"

extern u8 D_801455C8[]; /* @source 0x801455C8 @kind data */

/* @source 0x801D7EA0
 * @behavior Returns the index in the 60 stride-8 record entries at 0x801455C8 of
 * the ordinal-th (zero-based) entry whose presence byte is nonzero, or 0xFF when
 * the table holds fewer than ordinal + 1 such entries. Records are not modified.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 findActiveRecordSlot(u8 ordinal) {
    s32 matches;
    s32 index;

    matches = 0;
    for (index = 0; index < 0x3C; index++) {
        if (D_801455C8[index << 3] != 0) {
            if ((u8)matches == ordinal) {
                return (u8)index;
            }
            matches++;
        }
    }
    return 0xFF;
}
