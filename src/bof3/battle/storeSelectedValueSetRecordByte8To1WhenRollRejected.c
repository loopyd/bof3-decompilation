#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0378
 * @behavior Runs the battle selection roll func_800A3A10 for the current
 *   battler byte D_80146374 and selection byte D_80146394. When the roll
 *   rejects, marks byte +0x08 of the selection record pointer
 *   D_8014639C.record as 1; when it accepts, copies the selected battler's
 *   leading halfword - read from the 0x140-stride party work record above
 *   0x80145F18 for selection bytes below 3, otherwise from the 0x118-stride
 *   enemy work record above 0x801EB6C4 rebased by -3 - into the signed
 *   halfword at byte +0x04 of that record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeSelectedValueSetRecordByte8To1WhenRollRejected(void)
{
    u32 index;
    u32 enemy_index;
    u16 value;

    if ((u8)func_800A3A10(D_80146374, D_80146394) == 0) {
        /* Non-volatile view: the roll gate above already read the cell, and
         * widening the volatile u8 straight into the u32 costs an extra
         * `andi` on the load. storeHalvedSelectedValueWhenRollAccepted reads
         * the same cell through this view for the same reason. */
        index = *(u8 *)&D_80146394;
        if (index < 3) {
            value = *(u16 *)(D_80145F18 + index * 0x140);
        } else {
            enemy_index = index - 3;
            value = *(u16 *)(D_801EB6C4 + enemy_index * 0x118);
        }
        D_8014639C.record[2] = value;
    } else {
        ((u8 *)D_8014639C.record)[8] = 1;
    }
}
