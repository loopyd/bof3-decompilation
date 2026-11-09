#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FE0D0
 * @behavior Overlay handler reached only indirectly: no in-image jal targets the
 * address in this payload or in any of the 396 shipped payloads under
 * out/binaries, and the word occurs once as 0x801FE4A8 - 0-based word 7 of the
 * 22-word in-image code-pointer run at 0x801FE48C (0x801FDD7C ... 0x801FE38C)
 * terminated by the null word at 0x801FE4E4. That run's only consumer is
 * dispatchCoordinateLookupHandlerScenarioScena0600_801FDD14, which calls
 * func_801A7AD8(0x801FE478, 4), sign-extends the returned byte and calls the
 * run's word at that index through jalr with no arguments; the count 4 limits
 * reachable entries to words 0 through 3, so word 7 is never called in this
 * payload. It first re-arms the overlay effect group for ids 0, 1 and 5 through
 * func_80166E88(0, 1, 5, 0) - a0=0, a1=1 and a2=5 materialised before the jal
 * and a3 zeroed by addu $a3,$zero,$zero in that call's delay slot - then calls
 * the helper at 0x801C1400 with the constant argument 0 (a0 zeroed in its delay
 * slot), which resets the entry-0 front script/runtime bank, then the per-id
 * helper func_801C187C with the constant argument 1, then the same per-id
 * helper with the constant argument 5, and finally func_801C1630 with no
 * arguments (its delay slot a plain nop). It takes no arguments of its own,
 * returns nothing, reads no memory and writes none; the 0x18-byte frame only
 * keeps $ra across the five calls. The 76 bytes are not byte-identical to any
 * span of the 396 shipped payloads (unmasked scan, one hit: this span), so
 * there is no clone donor; they differ from the adjacent word-4 body
 * armEffectGroupThenInvokeFourHelpersScenarioScena0600_801FDFFC in exactly one
 * word, the immediate of the first per-id func_801C187C argument (1 here, 0
 * there), and they are the adjacent word-6 body
 * armEffectGroupThenInvokeThreeHelpersScenarioScena0600_801FE08C with the extra
 * func_801C187C(5) call inserted before its func_801C1630 tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena0600_801FE0D0(void) {
  func_80166E88(0, 1, 5, 0);
  func_801C1400(0);
  func_801C187C(1);
  func_801C187C(5);
  func_801C1630();
}
