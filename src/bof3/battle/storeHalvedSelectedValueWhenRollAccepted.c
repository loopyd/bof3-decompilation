#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0244
 * @behavior Runs the battle selection roll func_800A3A10 for the current
 *   battler byte D_80146374 and selection byte D_80146394. When the roll
 *   accepts, halves the selected battler's leading halfword - read from the
 *   0x140-stride party work record above 0x80145F18 for selection bytes below
 *   3, otherwise from the 0x118-stride enemy work record above 0x801EB6C4
 *   rebased by -3 - and stores the result in the signed halfword at byte
 *   +0x04 of the selection record pointer D_8014639C.record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeHalvedSelectedValueWhenRollAccepted(void)
{
    u32 index;
    u32 enemy_index;
    u16 value;

    if ((u8)func_800A3A10(D_80146374, D_80146394) != 0) {
        return;
    }
    /* Non-volatile view: the roll gate above already read the cell, and
     * letting the volatile u8 widen into the u32 itself costs an extra `andi`
     * on the load. func_8009DE8C reads the same cell through this view for the
     * same reason. */
    index = *(u8*)&D_80146394;
    if (index < 3) {
        value = *(u16*)(D_80145F18 + index * 0x140);
    } else {
        enemy_index = index - 3;
        value = *(u16*)(D_801EB6C4 + enemy_index * 0x118);
    }
    ((s16*)D_8014639C.record)[2] = value >> 1;
}
