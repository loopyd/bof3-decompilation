#include "bof3/world/area02613_internal.h"

/* @behavior clears bit 0x28 in the active scenario flags.
 * @source 0x801F3124
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearScenarioFlagBit28(void) {
  func_8015B5A8((void*)D_8014686C, 0x28u);
}
