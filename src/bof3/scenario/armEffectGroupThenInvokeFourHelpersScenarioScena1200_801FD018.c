#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FD018
 * @behavior Overlay per-record callback and entry 12, the word at 0x801FD26C,
 * of the nineteen-word code-pointer run based at 0x801FD23C: the selector
 * dispatchCoordinateLookupHandlerScenarioScena1200_801FCC54 (0x801FCC54)
 * passes the four five-byte lookup records at 0x801FD228 and the length 4 to
 * the resident lookup helper func_801A7AD8, reads that answer as a signed byte,
 * returns -1 when it is negative and otherwise calls the selected word of that
 * run with no arguments. Its four records select only indices 0 through 3, so
 * this entry is reached only through the run word itself. That run word is the
 * only in-image reference to this address: no jal and no j in the payload
 * targets it, and no other word equals it.
 *
 * It first re-arms the three-entry overlay effect group for ids 7, 4 and 2
 * through func_80166E88(7, 4, 2, 0) - a0=7, a1=4 and a2=2 materialised before
 * the jal and a3 zeroed by `addu $a3,$zero,$zero` in that call's delay slot -
 * then calls the helper at 0x801C1400 with the constant argument 0 (a0 zeroed
 * in its delay slot), then the per-id teardown helper func_801C187C with the
 * constant argument 4, then the same per-id helper again with the constant
 * argument 2, and finally func_801C1630 (its delay slot is a plain nop). It
 * takes no arguments of its own, reads no memory and writes none; the
 * 0x18-byte frame only keeps $ra across the five calls and every later constant
 * is materialised in the delay slot of its own call.
 *
 * Its 76 bytes compare equal, byte for byte over the whole span, to the span at
 * 0x801FADAC of emi/scenario/scena11/00, the exact sibling
 * armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FADAC, which carries
 * the same five calls with the same constants 7 / 4 / 2 / 0, 0, 4 and 2, so the
 * body was taken from that exact sibling while the address, the frame and the
 * immediates were confirmed against this target's own payload. The name records
 * that proven behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena1200_801FD018(void) {
  func_80166E88(7, 4, 2, 0);
  func_801C1400(0);
  func_801C187C(4);
  func_801C187C(2);
  func_801C1630();
}
