#include "bof3/bof3.h"

extern u8 D_8014496F[]; /* @source 0x8014496F @kind data */
extern void func_801D90D8(s32 x, s32 y, u8 record, u8 flag);

/* @source 0x801D9210
 * @behavior Draws the flagged field record list: scans the seven 0xA4-byte
 *           stride field records, tests bit 0 of the flag byte of each through
 *           the table at 0x8014496F and, for every flagged record in scan
 *           order, draws its entry through func_801D90D8 at the position
 *           0x20 + 0x88 * (ordinal & 1) by 0x2B + 0x38 * (ordinal >> 1),
 *           passing the record index truncated to a byte and a zero selector.
 *           Records are not modified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFlaggedFieldRecordList(void) {
    u8 count;
    u32 index;
    s32 offset;

    count = 0;
    for (index = 0; index < 7; index++) {
        offset = index * 0xA4;
        if ((D_8014496F[offset] & 1) != 0) {
            func_801D90D8((count & 1) * 0x88 + 0x20, (count >> 1) * 0x38 + 0x2B,
                          (u8)index, 0);
            count++;
        }
    }
}
