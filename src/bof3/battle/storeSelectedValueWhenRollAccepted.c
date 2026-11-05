#include "bof3/battle/battle15_internal.h"

/* @source 0x800A1B38
 * @behavior Runs the selection roll func_800A3C9C for the current battler byte
 *   D_80146374 and selection byte D_80146394. When the roll accepts, copies the
 *   selected battler's leading halfword - read from the 0x140-stride party work
 *   record above 0x80145F18 for selection bytes below 3, otherwise from the
 *   0x118-stride enemy work record above 0x801EB6C4 rebased by -3 - into the
 *   signed halfword at byte +0x04 of the selection record pointer
 *   D_8014639C.record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeSelectedValueWhenRollAccepted(void)
{
    u32 index;
    u32 enemy_index;

    if ((u8)func_800A3C9C(D_80146374, D_80146394) != 0) {
        return;
    }
    /* Non-volatile view: storeHalvedSelectedValueWhenRollAccepted and
     * func_8009DE8C read the same cell through it, which keeps the widening
     * from adding a trailing `andi` to the load. */
    index = *(u8*)&D_80146394;
    if (index < 3) {
        D_8014639C.record[2] = *(u16*)(D_80145F18 + index * 0x140);
    } else {
        enemy_index = index - 3;
        D_8014639C.record[2] = *(u16*)(D_801EB6C4 + enemy_index * 0x118);
    }
}
