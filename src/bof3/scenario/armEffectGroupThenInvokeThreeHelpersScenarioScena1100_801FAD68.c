#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FAD68
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 6 and 5 through func_80166E88(7, 6, 5, 0),
 * then calls the helper at 0x801C1400 with the constant argument 0, then the
 * per-id helper func_801C187C with the constant argument 6, and finally
 * func_801C1630. It takes no arguments, returns nothing, reads no memory of its
 * own and writes none; the 0x18-byte frame only keeps $ra across the four
 * calls. The dispatcher dispatchRecordCallbackScenarioScena1100_801FA5B8
 * invokes it indirectly through word 22 of the 29-entry overlay callback run at
 * 0x801FAF24 (0x801FAF7C holds 0x801FAD68), so the overlay selects it with the
 * record byte at offset 0x7A reading 0x16; the record pointer and second
 * argument that dispatcher passes are ignored, and no in-image jal reaches the
 * address. Its 68 bytes are the same arm-then-three-helpers shape as the exact
 * siblings armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FABC0
 * (0x801FABC0, ids 7, 8, 5 with func_801C187C(8)),
 * armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FAC94
 * (0x801FAC94, ids 7, 2, 5 with func_801C187C(5)) and
 * armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FACD8
 * (0x801FACD8, ids 7, 8, 5 with func_801C187C(5), byte-identical to
 * 0x801FAC04), differing only in the ids the arm call carries (7, 6, 5 here)
 * and in the func_801C187C argument (6 here); the address anchor keeps the
 * names target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FAD68(void) {
  func_80166E88(7, 6, 5, 0);
  func_801C1400(0);
  func_801C187C(6);
  func_801C1630();
}
