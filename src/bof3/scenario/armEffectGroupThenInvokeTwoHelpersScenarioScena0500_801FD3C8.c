#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);

/* @source 0x801FD3C8
 * @behavior Overlay handler reached only indirectly (no in-image `jal` targets
 * it): it first re-arms the overlay effect group for the ids 0, 1 and 5
 * through func_80166E88(0, 1, 5, 0) - a0=0, a1=1 and a2=5 materialised before
 * the jal and a3 zeroed in that call's delay slot - then calls the helper at
 * 0x801C1400 with the constant argument 0, and finally the per-id helper
 * func_801C187C with the constant argument 1. It takes no arguments and returns
 * nothing; the 0x18-byte frame only keeps $ra across the three calls and each
 * later constant is materialised in the delay slot of its own call. The address
 * is entry 8 - the word at 0x801FD694, payload offset 0x6A94 - of the
 * 18-entry overlay handler pointer table D_801FD674 (0x801FD674 ...
 * 0x801FD6B8, terminated by the null word at 0x801FD6BC) that sits inside the
 * Splat func_801FD5A8 blob, so the table selects this handler instead of a jal.
 * The 60 bytes are instruction-identical to the sibling
 * armEffectGroupThenInvokeTwoHelpersScenarioScena0500_801FD38C (entry 7 at
 * 0x801FD690) apart from the func_801C187C id immediate, which is 1 here and 5
 * there, and the call shape is the established exact family
 * armEffectGroupThenInvokeTwoHelpersScenarioScena0600_801FE15C (ids 0, 1 and 2
 * with the per-id helper id 1 there).
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeTwoHelpersScenarioScena0500_801FD3C8(void) {
  func_80166E88(0, 1, 5, 0);
  func_801C1400(0);
  func_801C187C(1);
}
