#include "bof3/bof3.h"

extern u32 D_8014686C;

void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FC7E8
 * @behavior Thin overlay wrapper setting flag bit 0x10 in the active scenario
 * flag word D_8014686C through the shared bit-set helper at 0x8015B580; takes
 * no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit10ScenarioScena0900_801FC7E8(void) {
  func_8015B580(D_8014686C, 0x10);
}
