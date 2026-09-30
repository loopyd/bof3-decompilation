#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FAC94
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 2 and 5 through func_80166E88(7, 2, 5, 0),
 * then calls the helper at 0x801C1400 with the constant argument 0, then the
 * per-id helper func_801C187C with the constant argument 5, and finally
 * func_801C1630. It takes no arguments, returns nothing, reads no memory of its
 * own and writes none; the 0x18-byte frame only keeps $ra across the four
 * calls. The dispatcher dispatchRecordCallbackScenarioScena1100_801FA5B8
 * invokes it indirectly through word 19 of the 29-entry overlay callback run at
 * 0x801FAF24 (0x801FAF70 holds 0x801FAC94), so the overlay selects it with the
 * record byte at offset 0x7A reading 0x13; the record pointer and second
 * argument that dispatcher passes are ignored, and no in-image jal reaches the
 * address. The 68 bytes differ from func_801FACD8 (0x801FACD8) and from
 * func_801FAC04 (0x801FAC04), which are byte-identical to each other, in
 * exactly one byte, the immediate of the second func_80166E88 argument (id 2
 * here against id 8 there); the address anchor keeps the names
 * target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeThreeHelpersScenarioScena1100_801FAC94(void) {
  func_80166E88(7, 2, 5, 0);
  func_801C1400(0);
  func_801C187C(5);
  func_801C1630();
}
