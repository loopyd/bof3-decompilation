#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FABC0
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 8 and 5 through func_80166E88(7, 8, 5, 0),
 * then calls the helper at 0x801C1400 with the constant argument 0, then the
 * per-id helper func_801C187C with the constant argument 8, and finally
 * func_801C1630. It takes no arguments, returns nothing, reads no memory of its
 * own and writes none; the 0x18-byte frame only keeps $ra across the four
 * calls. The dispatcher dispatchRecordCallbackScenarioScena1100_801FA5B8
 * invokes it indirectly through word 16 of the 29-entry overlay callback run at
 * 0x801FAF24 (0x801FAF64 holds 0x801FABC0), so the overlay selects it with the
 * record byte at offset 0x7A reading 0x10; the record pointer and second
 * argument that dispatcher passes are ignored, and no in-image jal reaches the
 * address. The 68 bytes differ from the immediate neighbour func_801FAC04
 * (0x801FAC04) in exactly one byte, the immediate of the func_801C187C argument
 * (id 8 here against id 5 there), and func_801FAB74 (0x801FAB74) runs the same
 * arm call and helper tail with one extra func_801C187C(5) call before
 * func_801C1630; the address anchor keeps the names target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FABC0(void) {
  func_80166E88(7, 8, 5, 0);
  func_801C1400(0);
  func_801C187C(8);
  func_801C1630();
}
