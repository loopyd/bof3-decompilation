#include "bof3/bof3.h"

extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801FA12C
 * @behavior Entry 2 of the four-word primary-state request handler run at
 * 0x801FA230 (0x801FA074, 0x801FA0B4, 0x801FA12C, 0x801FA178) that
 * dispatchRoute30CoordinateLookupHandlerScenarioScena0400_801FA010
 * (0x801FA010) indexes with the sign-extended byte func_801A7AD8(&D_801FA228,
 * 1, ...) returns, so only that indirect call reaches it and no in-image jal
 * targets it. It first re-arms the overlay effect group for the ids 0, 1 and 5
 * through func_80166E88(0, 1, 5, 0) - a0 and a3 both zeroed with addu, a1=1 and
 * a2=5 materialised with addiu before the jal and a3 zeroed in that call's
 * delay slot - then clears the shared live work-record count byte D_80146254
 * with a store of $zero, and finally runs the per-id effect teardown
 * func_801C187C for the ids 0, 1 and 5 in that order, each id materialised in
 * $a0 inside its own jal delay slot. It takes no arguments, consumes no result
 * and nothing sets $v0; the 0x18-byte frame only keeps $ra at 0x10 across the
 * four calls. Its immediate neighbour 0x801FA0B4 carries the same
 * arm-clear-teardown shape with a six-byte record invalidation inserted
 * between the arm and the clear. The 76-byte span is byte-identical to
 * clearWorkRecordsAndResetEffectSlots_scena18 (0x801F6D04 of
 * emi/scenario/scena18/00, whose payload also loads at 0x801F6C00) in every
 * word except the four constant immediates that carry the arm ids (0, 3, 4
 * there) and the teardown ids (0, 3, 4 there), so the statement order, operand
 * forms and local declaration style came from that sibling and this span's own
 * literals come from its bytes; a scan of every extracted payload for this
 * span's first 32 bytes finds no byte-identical copy, so no clone applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupClearCountAndResetEffectSlots0And1And5ScenarioScena0400_801FA12C(void) {
  func_80166E88(0, 1, 5, 0);
  D_80146254 = 0;
  func_801C187C(0);
  func_801C187C(1);
  func_801C187C(5);
}
