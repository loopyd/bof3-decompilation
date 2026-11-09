#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FDFFC
 * @behavior Overlay handler reached only indirectly: no in-image `jal` targets
 * the address, whose word is stored at 0x801FE49C - 0-based word 4 of the
 * 22-word in-image code-pointer run at 0x801FE48C (0x801FDD7C ... 0x801FE38C,
 * terminated by the null word at 0x801FE4E4) - so the overlay selects and calls
 * it through a pointer. That run's only consumer is
 * dispatchCoordinateLookupHandlerScenarioScena0600_801FDD14, which sizes the
 * sibling record table at 0x801FE478 with the length 4 (four five-byte records,
 * 20 bytes ending on the run boundary), so only entries 0 through 3 are
 * reachable and this entry 4 is never called in this payload. It first re-arms
 * the overlay effect group for ids 0, 1 and 5 through func_80166E88(0, 1, 5, 0)
 * - a0=0, a1=1 and a2=5 materialised before the jal and a3 zeroed by
 * `addu $a3,$zero,$zero` in that call's delay slot - then calls the helper at
 * 0x801C1400 with the constant argument 0 (a0 zeroed in its delay slot), which
 * resets the entry-0 front script/runtime bank, then the per-id helper
 * func_801C187C with the constant argument 0, then the same per-id helper again
 * with the constant argument 5, and finally func_801C1630 with no arguments
 * (its delay slot a plain nop). It takes no arguments of its own, returns
 * nothing, reads no memory and writes none; the 0x18-byte frame only keeps $ra
 * across the five calls. The 76 bytes are not byte-identical to any span of the
 * 396 shipped payloads under out/binaries (unmasked scan, one hit: this span),
 * so there is no clone donor; they are the five-call shape of the exact scena09
 * sibling armEffectGroupThenInvokeFourHelpersScenarioScena0900_801FDD7C
 * (0x801FDD7C), differing only in the effect-group ids (0, 1, 5 here, 7, 8, 5
 * there) and the first per-id argument (0 here, 8 there), and they are the
 * three-helper body of the adjacent in-image entry 2
 * armEffectGroupThenInvokeThreeHelpersScenarioScena0600_801FE08C with the extra
 * func_801C187C(0) call inserted before its func_801C187C(5).
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena0600_801FDFFC(void) {
  func_80166E88(0, 1, 5, 0);
  func_801C1400(0);
  func_801C187C(0);
  func_801C187C(5);
  func_801C1630();
}
