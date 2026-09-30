#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC7D4
 * @behavior Runs the shared front-end startup helper func_8015C088 and then
 * requests primary scene state 1 with secondary sub-state 0x0A: it stores 0x0A
 * in the shared sub-state byte D_80146875 and then 1 in the shared state byte
 * D_80146874, the same pair and the same store shape the sibling scenario lifts
 * requestPrimaryState17AndClearSubstateScenarioScena0200_801FD574 and
 * requestPrimaryState6Substate10ScenarioScena0700_801FDAFC use for the request
 * "primary state N, sub-state M". It takes no arguments and returns nothing,
 * and its 0x18-byte frame only keeps $ra across the call; each of the two byte
 * stores materialises the 0x8014 page in $at on its own, which is how the psyq
 * compiler emits separate shared-byte stores. No in-image jal targets the
 * address; it is the last code pointer of the table at 0x801FD1B8 (the word at
 * 0x801FD268, entry 43 of the 44 pointer words 0x801F7F64 through 0x801FC7D4),
 * which the overlay's progress dispatcher at 0x801F7F28 selects with the signed
 * shared scenario state byte D_80146872, so this step is that chain's final
 * entry: it leaves the scene by requesting primary state 1 / sub-state 0x0A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState1Substate10ScenarioScena1200_801FC7D4(void) {
  func_8015C088();
  D_80146875 = 0xA;
  D_80146874 = 1;
}
