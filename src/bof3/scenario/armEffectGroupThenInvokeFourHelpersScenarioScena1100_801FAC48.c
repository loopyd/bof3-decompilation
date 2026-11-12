#include "bof3/bof3.h"

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C1400(u32 arg0);
void func_801C187C(s32 arg0);
void func_801C1630(void);

/* @source 0x801FAC48
 * @behavior Overlay per-record callback: it first re-arms the three-entry
 * overlay effect group for ids 7, 8 and 5 through func_80166E88(7, 8, 5, 0) -
 * a0=7, a1=8 and a2=5 materialised before the jal and a3 zeroed by `addu
 * $a3,$zero,$zero` in that call's delay slot - then calls the helper at
 * 0x801C1400 with the constant argument 0 (a0 zeroed in its delay slot), then
 * the per-id helper func_801C187C with the constant argument 8, then the same
 * per-id helper again with the constant argument 5, and finally func_801C1630
 * (its delay slot is a plain nop). It takes no arguments, returns nothing,
 * reads no memory of its own and writes none; the 0x18-byte frame only keeps
 * $ra across the five calls and each later constant is materialised in the
 * delay slot of its own call. The dispatcher
 * dispatchRecordCallbackScenarioScena1100_801FA5B8 invokes it indirectly
 * through word 18 of the 29-entry overlay callback run at 0x801FAF24
 * (0x801FAF6C holds 0x801FAC48), so the record byte at offset 0x7A selects it
 * with the value 0x12; the record pointer and second argument that dispatcher
 * passes are ignored, and neither a jal nor a j anywhere in the image reaches
 * the address. Its 76 bytes are byte-for-byte identical to the exact neighbour
 * armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FAB74 (0x801FAB74),
 * the same arm call and the same four-helper tail; the address anchor keeps
 * the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupThenInvokeFourHelpersScenarioScena1100_801FAC48(void) {
  func_80166E88(7, 8, 5, 0);
  func_801C1400(0);
  func_801C187C(8);
  func_801C187C(5);
  func_801C1630();
}
