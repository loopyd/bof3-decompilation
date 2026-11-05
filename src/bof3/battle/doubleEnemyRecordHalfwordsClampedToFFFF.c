#include "bof3/battle/battle15_internal.h"

/* @source 0x800A07D0
 * @behavior Selection-action handler, entry 95 of the battle selection-action
 *   table D_800B471C. Runs the selection-input refresh pass func_800A3F28, then
 *   doubles the halfwords at +0x00 and +0x02 of the 0x118-byte enemy-band work
 *   record field base D_801EB6B4/D_801EB6B6 - the record selected by the active
 *   battler byte D_80146394 rebased by -3, which is the same band and rebase
 *   storeHalvedSelectedValueWhenRollAccepted reads at 0x801EB6C4 - saturating
 *   each doubled value at 0xFFFF and storing both back. The battler byte is
 *   read once for the two loads and again after the first store; that is why
 *   the original emits two `lbu` reads and two separate 0x118 scale chains.
 * @status exact
 * @match 100.00
 * @residual none
 */
void doubleEnemyRecordHalfwordsClampedToFFFF(void)
{
    volatile u8 *selected;
    u32 offset;
    u32 offset2;
    u16 first;
    u16 second;
    s32 doubled_first;
    s32 doubled_second;

    func_800A3F28();
    selected = &D_80146394;
    offset = (u32)(*selected - 3) * 0x118;
    first = *(u16 *)(D_801EB6B4 + offset);
    doubled_first = first * 2;
    second = *(u16 *)(D_801EB6B6 + offset);
    first = (u16)doubled_first;
    if (doubled_first > 0xFFFF) {
        first = 0xFFFF;
    }
    doubled_second = second * 2;
    second = (u16)doubled_second;
    if (doubled_second > 0xFFFF) {
        second = 0xFFFF;
    }
    *(u16 *)(D_801EB6B4 + offset) = first;
    offset2 = (u32)(*selected - 3) * 0x118;
    *(u16 *)(D_801EB6B6 + offset2) = second;
}
