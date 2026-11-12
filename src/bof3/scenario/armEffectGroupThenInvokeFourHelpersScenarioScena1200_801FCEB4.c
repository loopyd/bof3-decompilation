#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FCEB4
 * @behavior Overlay per-record callback and entry 7, the word at 0x801FD258, of
 * the nineteen-word code-pointer run based at 0x801FD23C: the selector
 * dispatchCoordinateLookupHandlerScenarioScena1200_801FCC54 (0x801FCC54)
 * passes the four five-byte lookup records at 0x801FD228 and the length 4 to
 * the resident lookup helper func_801A7AD8, reads that answer as a signed byte,
 * returns -1 when it is negative and otherwise indexes that run with the byte
 * and calls the selected word with jalr, returning its signed low byte. That
 * run word is the only in-image reference to this address; neither a jal nor a
 * j in the payload names it. It first re-arms the three-entry overlay effect
 * group for ids 7, 8 and 5 through func_80166E88(7, 8, 5, 0) - a0=7, a1=8 and
 * a2=5 materialised before the jal and a3 zeroed by `addu $a3,$zero,$zero` in
 * that call's delay slot - then calls the helper at 0x801C1400 with the
 * constant argument 0 (a0 zeroed in its delay slot), then the per-id teardown
 * helper func_801C187C with the constant argument 8, then the same per-id
 * helper again with the constant argument 5, and finally func_801C1630 (its
 * delay slot is a plain nop). It takes no arguments of its own, reads no memory
 * and writes none; the 0x18-byte frame only keeps $ra across the five calls and
 * every later constant is materialised in the delay slot of its own call. Its
 * 76 bytes are byte-identical to the exact sibling
 * armEffectGroupThenInvokeFourHelpersScenarioScena1200_801FCDE0 at 0x801FCDE0
 * of this target, proved by comparing the two 76-byte payload spans byte for
 * byte; the sibling sits in the same code-pointer run as entry 4 and carries
 * the same five calls with the same constants 7 / 8 / 5 / 0, 0, 8 and 5, so
 * only the run entry and the address differ. The name records that proven
 * behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena1200_801FCEB4(void) {
  func_80166E88(7, 8, 5, 0);
  func_801C1400(0);
  func_801C187C(8);
  func_801C187C(5);
  func_801C1630();
}
