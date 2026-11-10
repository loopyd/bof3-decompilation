#include "base/types.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF6CC
 * @behavior Reads the shared work pointer once. When work byte 0xB is below
 *           2, decrements it at byte width and calls the helper at 0x801E5988.
 *           Zero wraps to 0xFF; values from 2 through 0xFF do nothing.
 *           Takes no arguments and returns nothing. Field roles are structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperWhenBelow2BmagicMagic00303_801EF6CC(void) {
    u8 *work = D_801EB4E0;

    if (work[0xB] < 2) {
        work[0xB]--;
        func_801E5988();
    }
}
