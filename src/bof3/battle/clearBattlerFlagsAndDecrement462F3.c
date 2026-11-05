#include "bof3/battle/battle03_internal.h"

void func_80196718(void* arg0); /* @source 0x80196718 */
void func_800ACFF4(void); /* @source 0x800ACFF4 */

/* @source 0x801E51DC
 * @behavior Clears the scratch work battler's action flags, decrements byte
 * D_801462F3 and sets D_80146328 bit 1 when it reaches zero, submits the work
 * through func_80196718, clears the battler's pending bit, stores 2 in
 * D_801483C3, deactivates dispatch slots and calls func_800ACFF4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearBattlerFlagsAndDecrement462F3(void)
{
    u8* counter;

    clearBattlerActionFlags(D_1F800044->unk_05);
    counter = &D_801462F3;
    if (--*counter == 0) {
        D_80146328 |= 2;
    }
    func_80196718(D_1F800044);
    clearPendingBit(D_1F800044->unk_05);
    D_801483C3 = 2;
    deactivateBattleDispatchSlots();
    func_800ACFF4();
}
