#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FAD1C
 * @behavior Overlay per-record callback reached only indirectly: the address is
 * word 21 (the word stored at 0x801FAF78) of the 29-entry in-image code-pointer
 * run at 0x801FAF24 that dispatchRecordCallbackScenarioScena1100_801FA5B8
 * (0x801FA5B8) indexes with the record unsigned byte at offset 0x7A and calls
 * with jalr, so a record byte reading 0x15 selects it, and no in-image jal or j
 * targets the address. It first re-arms the overlay effect group for the ids 7,
 * 6 and 5 through func_80166E88(7, 6, 5, 0) - a0=7, a1=6 and a2=5 materialised
 * before the jal and a3 zeroed by addu in that call's delay slot - then calls
 * the helper at 0x801C1400 with the constant argument 0 (a0 zeroed in its delay
 * slot), then the per-id helper func_801C187C with the constant argument 5,
 * then the same per-id helper again with the constant argument 6, and finally
 * func_801C1630 (its delay slot is a plain nop). It takes no arguments, returns
 * nothing, reads no memory of its own and writes none; the 0x18-byte frame only
 * keeps $ra across the five calls and each later constant is materialised in
 * the delay slot of its own call. A payload comparison of the 76 bytes against
 * the gate-exact sibling armEffectGroupThenInvokeFourHelpersScenarioScena0900_801FDE80
 * (0x801FDE80) of emi/scenario/scena09/00 proved the two spans byte-for-byte
 * equal, so the body is shared and only the address anchor keeps the names
 * target-qualified; the same 76 bytes were also equal at 0x801FCF88 of
 * emi/scenario/scena12/00, whose own row still held that entry as assembly when
 * the comparison ran. The nearer neighbours in this target carry the same
 * arm-then-helpers shape with other ids: armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FAC48
 * (0x801FAC48, ids 7, 8, 5 with func_801C187C(8) then (5)) differs only in the
 * three stored immediates, and func_801FADAC (0x801FADAC, ids 7, 4, 2 with
 * func_801C187C(4) then (2)) differs only in four.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FAD1C(void) {
  func_80166E88(7, 6, 5, 0);
  func_801C1400(0);
  func_801C187C(5);
  func_801C187C(6);
  func_801C1630();
}
