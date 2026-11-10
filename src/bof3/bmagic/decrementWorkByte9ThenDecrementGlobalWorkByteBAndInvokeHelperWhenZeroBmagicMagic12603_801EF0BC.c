#include "bof3/bof3.h"

extern void func_801E5988(void); /* @source 0x801E5988 */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* @source 0x801EF0BC
 * @behavior Decrements scratchpad work-object byte 9 modulo 256. When the
 * resulting byte is zero, decrements byte 0xB of the shared record reached
 * through 0x801EB4E0 and calls the helper at 0x801E5988. The handler table
 * based at 0x801F125C holds this entry at 0x801F1274. Takes no arguments and
 * returns nothing. The original 92-byte span compared equal to
 * magic110/03 at 0x801EF93C; its source supplied the statement shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic12603_801EF0BC(void) {
    if (--D_1F800044[0x09] == 0) {
        D_801EB4E0[0x0B]--;
        func_801E5988();
    }
}
