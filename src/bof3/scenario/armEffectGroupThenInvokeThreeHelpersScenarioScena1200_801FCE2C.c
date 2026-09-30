#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FCE2C
 * @behavior Overlay callback selected by the selector func_801FCC54
 * (0x801FCC54): that selector passes the four-byte overlay record at
 * 0x801FD228 to func_801A7AD8 together with the length 4, indexes the
 * code-pointer run at 0x801FD23C with the signed byte the helper returns and
 * calls the selected word with jalr, returning its sign-extended byte; the word
 * at 0x801FD250, index 5 of that run, is this address, its only in-image
 * reference. It first re-arms the three-entry overlay effect group for ids 7, 8
 * and 5 through func_80166E88(7, 8, 5, 0), then calls the helper at 0x801C1400
 * with the constant argument 0, then the per-id teardown helper func_801C187C
 * with the constant argument 8, and finally func_801C1630. It takes no
 * arguments of its own, reads no memory and writes none; the 0x18-byte frame
 * only keeps $ra across the four calls. The 68 bytes are byte-identical to the
 * exact sibling armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FABC0
 * at 0x801FABC0 of emi/scenario/scena11/00, which carries the same four calls
 * with the same constants 7 / 8 / 5 / 0, 0, 8 (compared directly in both
 * shipped payloads), and the neighbouring word 0x801FCDE0 of this overlay is
 * byte-identical to the scena11 sibling at 0x801FAB74, the same arm call and
 * helper tail with one extra func_801C187C(5). The name records that proven
 * behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena1200_801FCE2C(void) {
  func_80166E88(7, 8, 5, 0);
  func_801C1400(0);
  func_801C187C(8);
  func_801C1630();
}
