#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FCF88
 * @behavior Overlay per-record callback reached only indirectly: the address is
 * entry 10 (the word stored at 0x801FD264) of the nineteen-word in-image
 * code-pointer run at 0x801FD23C that
 * callCoordinateRecordSelect (0x801FCC54)
 * indexes with the signed byte func_801A7AD8 returns for the four five-byte
 * lookup records at 0x801FD228 and the length 4; that resident helper returns
 * the record index, so the selector reaches only entries 0 through 3 of the
 * run. The word at 0x801FD264 is the only in-image reference to this address
 * and no jal or j targets it. It first re-arms the overlay effect group for the
 * ids 7, 6 and 5 through func_80166E88(7, 6, 5, 0) - a0=7, a1=6 and a2=5
 * materialised before the jal and a3 zeroed by addu in that call's delay slot -
 * then calls the helper at 0x801C1400 with the constant argument 0 (a0 zeroed
 * in its delay slot), then the per-id helper func_801C187C with the constant
 * argument 5, then the same per-id helper again with the constant argument 6,
 * and finally func_801C1630 (its delay slot is a plain nop). It takes no
 * arguments, returns nothing, reads no memory of its own and writes none; the
 * 0x18-byte frame only keeps $ra across the five calls and each later constant
 * is materialised in the delay slot of its own call. A payload comparison
 * proved its 76 bytes byte-for-byte equal to the gate-exact siblings
 * armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FAD1C (0x801FAD1C) of
 * emi/scenario/scena11/00 and
 * armEffectGroupThenInvokeFourHelpersScenarioScena0900_801FDE80 (0x801FDE80) of
 * emi/scenario/scena09/00, and those three payloads are the only ones holding
 * that span, so the body was cloned from them. The nearer neighbour
 * armEffect85TearBoth2 (0x801FCEB4,
 * ids 7, 8, 5 with func_801C187C(8) then (5)) of this target differs only in
 * the three stored immediates. The name records that proven behavior and the
 * address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffect65TearBoth(void) {
  func_80166E88(7, 6, 5, 0);
  func_801C1400(0);
  func_801C187C(5);
  func_801C187C(6);
  func_801C1630();
}
