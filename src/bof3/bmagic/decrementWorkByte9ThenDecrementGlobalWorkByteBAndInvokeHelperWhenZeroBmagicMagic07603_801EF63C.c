#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF63C
 * @behavior Callback at table word 0x801F0498, indexed by the work-byte-1
 *           dispatcher at 0x801EF330. Decrements scratchpad work byte 9 and
 *           tests the truncated byte result. When it reaches zero, decrements
 *           byte 0xB of the record read through pointer cell 0x801EB4E0 and
 *           invokes the helper at 0x801E5988. Takes no arguments and returns
 *           nothing. Each pointer cell is read once.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic07603_801EF63C(void) {
    if (--g_battle_work[0x09] == 0) {
        D_801EB4E0[0x0B] -= 1;
        func_801E5988();
    }
}
