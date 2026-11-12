#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FADAC
 * @behavior Overlay per-record callback reached only indirectly: the address is
 * word 23 (the word stored at 0x801FAF80) of the 29-entry overlay callback run
 * based at 0x801FAF24 that dispatchRecordCallbackScenarioScena1100_801FA5B8
 * (0x801FA5B8) indexes with the record unsigned byte at offset 0x7A and calls
 * with jalr, so a record byte reading 0x17 selects it, and no jal or j in the
 * payload reaches the address. It first re-arms the overlay effect group for
 * the ids 7, 4 and 2 through func_80166E88(7, 4, 2, 0) - a0=7, a1=4 and a2=2
 * materialised before the jal and a3 zeroed by addu in that call's delay slot -
 * then calls the helper at 0x801C1400 with the constant argument 0 (a0 zeroed
 * in its delay slot), then the per-id helper func_801C187C with the constant
 * argument 4, then the same per-id helper again with the constant argument 2,
 * and finally func_801C1630 (its delay slot is a plain nop). It takes no
 * arguments, returns nothing, reads no memory of its own and writes none; the
 * 0x18-byte frame only keeps $ra across the five calls and each later constant
 * is materialised in the delay slot of its own call.
 *
 * A payload comparison of the 76 bytes against the span at 0x801FD018 of
 * emi/scenario/scena12/00 proved the two spans byte-for-byte equal, but that
 * target's own row still held the span as assembly when the comparison ran, so
 * the body here was transcribed from the nearest exact sibling instead:
 * armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FAD1C (0x801FAD1C,
 * ids 7, 6, 5 with func_801C187C(5) then (6)) differs only in the five stored
 * immediates, and armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FAC48
 * (0x801FAC48, ids 7, 8, 5 with func_801C187C(8) then (5)) in the same five.
 * The address anchor keeps the names target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FADAC(void) {
  func_80166E88(7, 4, 2, 0);
  func_801C1400(0);
  func_801C187C(4);
  func_801C187C(2);
  func_801C1630();
}
