#include "bof3/bof3.h"

extern u8 D_8014496F[]; /* @source 0x8014496F @kind data */

/* @source 0x801D92F4
 * @behavior Returns the record position of the ordinal-th (zero-based) field
 *           record that carries the flag bit, scanning the seven 0xA4-byte
 *           stride entries of the field record table through their flag bytes
 *           at 0x8014496F and testing bit 0 of each. Returns 0xFF when fewer
 *           than ordinal + 1 records are flagged. Records are not modified.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 findFlaggedFieldRecordSlot(u8 ordinal) {
    s32 matches;
    s32 index;
    s32 offset;

    matches = 0;
    for (index = 0; index < 0x7; index++) {
        offset = index * 0xA4;
        if ((D_8014496F[offset] & 1) != 0) {
            if ((u8)matches == ordinal) {
                return (u8)index;
            }
            matches++;
        }
    }
    return 0xFF;
}
