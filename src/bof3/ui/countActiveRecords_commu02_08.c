#include "bof3/bof3.h"

extern u8 D_801455C8[]; /* @source 0x801455C8 @kind data */

/* @source 0x801D7E64
 * @behavior Counts the nonzero presence bytes in the 60 stride-8 active records
 * at 0x801455C8. Returns the count as an unsigned byte without modifying records.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 countActiveRecords_commu02_08(void) {
    u8 count;
    s32 offset;

    count = 0;
    for (offset = 0; offset < 0x1E0; offset += 8) {
        if (D_801455C8[offset] != 0) {
            count++;
        }
    }
    return count;
}
