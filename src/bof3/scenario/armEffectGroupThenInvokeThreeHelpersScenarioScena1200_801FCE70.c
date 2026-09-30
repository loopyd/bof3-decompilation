#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FCE70
 * @behavior Overlay per-record callback and entry 6 (the word at 0x801FD254) of
 * the code-pointer run at 0x801FD23C that the selector func_801FCC54
 * (0x801FCC54) indexes with the signed byte func_801A7AD8 returns for the
 * four-byte overlay record at 0x801FD228; that run word is the only in-image
 * reference to this address and no in-image jal targets it. It first re-arms
 * the three-entry overlay effect group for ids 7, 8 and 5 through
 * func_80166E88(7, 8, 5, 0), then calls the helper at 0x801C1400 with the
 * constant argument 0, then the per-id teardown helper func_801C187C with the
 * constant argument 5, and finally func_801C1630. It takes no arguments of its
 * own, reads no memory and writes none; the 0x18-byte frame only keeps $ra
 * across the four calls. The 68 bytes are byte-identical to the exact siblings
 * armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FAC04 at 0x801FAC04
 * and armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FACD8 at
 * 0x801FACD8 of emi/scenario/scena11/00, which carry the same four calls with
 * the same constants 7 / 8 / 5 / 0, 0 and 5, compared directly in both shipped
 * payloads. The name records that proven behavior and the address anchor keeps
 * it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena1200_801FCE70(void) {
  func_80166E88(7, 8, 5, 0);
  func_801C1400(0);
  func_801C187C(5);
  func_801C1630();
}
