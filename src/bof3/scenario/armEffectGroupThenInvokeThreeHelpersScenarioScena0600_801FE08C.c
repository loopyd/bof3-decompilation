#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE08C
 * @behavior Overlay handler reached only indirectly: no in-image `jal` targets
 * the address, whose word is stored at 0x801FE494 - 0-based word 2 of the
 * in-image code-pointer run at 0x801FE48C (0x801FDFFC ... 0x801FE38C, 18
 * non-null words terminated by the null word at 0x801FE4D4) - so the overlay
 * selects and calls it through a pointer. It first re-arms the overlay effect
 * group for ids 0, 1 and 5 through func_80166E88(0, 1, 5, 0) - a0=0, a1=1 and
 * a2=5 materialised before the jal and a3 zeroed by `addu $a3,$zero,$zero` in
 * that call's delay slot - then calls the helper at 0x801C1400 with the
 * constant argument 0 (a0 zeroed in its delay slot), then the per-id helper
 * func_801C187C with the constant argument 1, and finally func_801C1630 with no
 * arguments (its delay slot a plain nop). It takes no arguments of its own,
 * returns nothing, reads no memory and writes none; the 0x18-byte frame only
 * keeps $ra across the four calls, and each later constant is materialised by
 * its own instruction before or in the delay slot of its jal. Its 68 bytes are
 * byte-identical to the exact sibling
 * armEffectGroupThenInvokeThreeHelpersScenarioScena0600_801FE048 (0x801FE048,
 * the immediately preceding word at 0x801FE490 of the same pointer run) except
 * for one byte, the immediate of the func_801C187C argument (1 here, 5 there),
 * so the address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena0600_801FE08C(void) {
  func_80166E88(0, 1, 5, 0);
  func_801C1400(0);
  func_801C187C(1);
  func_801C1630();
}
