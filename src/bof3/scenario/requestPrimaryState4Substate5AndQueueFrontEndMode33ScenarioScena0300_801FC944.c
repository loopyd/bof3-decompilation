#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
s32 queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(s32 arg0);

/* @source 0x801FC944
 * @behavior Route 0x25 handler of this overlay: the dispatcher at 0x801FC704
 * reaches it through the jal at 0x801FC77C, forwarding both incoming packed
 * record words unchanged, so the body saves $a0 in $s1 and $a1 in $s0 before
 * any call and never writes either. It first waits through the shared
 * two-stage helper func_8015B5D4(D_8014686C, 0x15); when that reports nonzero
 * it returns 0. It then requires the second packed record word to equal the
 * literal 0x418000 (lui/ori pair, so the comparison is against a built
 * constant and not a table entry) and returns 0 when it differs. Finally it
 * tests the first word's high half with the `addiu -0x20` / `sltiu 4` pair,
 * that is the inclusive range 0x20..0x23, and returns 0 outside it.
 * On that path it requests primary scene state 4 with sub-state 5 by storing 4
 * in the shared state byte D_80146874 and then 5 in the shared sub-state byte
 * D_80146875, queues front-end mode 0x33 through the overlay tail at
 * 0x801FCC9C and returns that tail's result sign-extended from its low byte
 * (sll/sra 24).
 * The 0x98 bytes use a 0x20-byte frame that keeps $s0, $s1 and $ra across the
 * two calls: the D_8014686C pointer load is scheduled between the second
 * saved-argument copy and the $ra store, and the constant 4 of the first state
 * store fills the delay slot of the range test's branch. No image word of this
 * target holds 0x801FC944, so the dispatcher's jal is its only reference.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestPrimaryState4Substate5AndQueueFrontEndMode33ScenarioScena0300_801FC944(s32 arg0, s32 arg1) {
  if (func_8015B5D4(D_8014686C, 0x15) != 0) {
    return 0;
  }

  if (arg1 != 0x418000) {
    return 0;
  }

  if ((u32)((arg0 >> 16) - 0x20) >= 4u) {
    return 0;
  }

  D_80146874 = 4;
  D_80146875 = 5;
  return (s8)queueFrontEndModeAndRequestPrimaryMode2ScenarioScena0300_801FCC9C(0x33);
}
