#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FD064
 * @behavior Overlay per-record callback and entry 13, the word at 0x801FD270, of
 * the nineteen-word code-pointer run based at 0x801FD23C: the selector
 * dispatchCoordinateLookupHandlerScenarioScena1200_801FCC54 (0x801FCC54)
 * passes the four five-byte lookup records at 0x801FD228 and the length 4 to
 * the resident lookup helper func_801A7AD8, reads that answer as a signed byte,
 * returns -1 when it is negative and otherwise indexes that run with the byte
 * and calls the selected word with jalr, returning its signed low byte. Those
 * four records select only indices 0 through 3, so this entry is reached only
 * through its run word. That run word is the only in-image reference to this
 * address: no jal and no j in the payload targets it, and no other word equals
 * it.
 *
 * It first re-arms the three-entry overlay effect group for ids 7, 5 and 2
 * through func_80166E88(7, 5, 2, 0) - a0=7, a1=5 and a2=2 materialised before
 * the jal and a3 zeroed by `addu $a3,$zero,$zero` in that call's delay slot -
 * then calls the helper at 0x801C1400 with the constant argument 0 (a0 zeroed
 * in its delay slot), then the per-id teardown helper func_801C187C with the
 * constant argument 2, then the same per-id helper again with the constant
 * argument 5, and finally func_801C1630 (its delay slot is a plain nop). It
 * takes no arguments of its own, reads no memory and writes none; the
 * 0x18-byte frame only keeps $ra across the five calls and every later constant
 * is materialised in the delay slot of its own call.
 *
 * Its 76 bytes differ from the exact sibling
 * armEffectGroupThenInvokeFourHelpersScenarioScena1200_801FD018 at 0x801FD018
 * of this target, proved by comparing the two 76-byte payload spans with their
 * carried-constant immediate bytes masked, in exactly the three constants it
 * passes: a1 reads 5 here against 4 there, and the two func_801C187C arguments
 * read 2 then 5 here against 4 then 2 there. Every call target of both spans
 * resolves in the shared main executable, so those carried constants are the
 * only words that can differ across the family. A scan of every extracted payload under
 * out/binaries for this exact 76-byte span found only its own offset, and the
 * same scan masked at those constants found the family members at
 * 0x801FCDE0, 0x801FCEB4, 0x801FCF88, 0x801FD018 and their scena09 and scena11
 * twins, none of which carries 5 / 2 / 2 / 5, so clone is ineligible and the
 * exact sibling supplied this body's statement order, operand forms and
 * declaration style while the five calls, their constants, the frame and the
 * address were written from this target's own payload. The name records that
 * proven behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena1200_801FD064(void) {
  func_80166E88(7, 5, 2, 0);
  func_801C1400(0);
  func_801C187C(2);
  func_801C187C(5);
  func_801C1630();
}
