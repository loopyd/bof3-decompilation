#include "bof3/bof3.h"

extern u8 D_80145029;

void func_80161D74(s32 arg0);

/* @source 0x801F4374
 * @behavior Thin overlay wrapper: passes the shared main-RAM byte D_80145029,
 * loaded unsigned, as the single argument of the shared helper at 0x80161D74
 * and returns. The load is materialised before the stack frame because the
 * argument is its only consumer. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperByteArgWorld03Area14514_801F4374(void) {
  func_80161D74(D_80145029);
}
