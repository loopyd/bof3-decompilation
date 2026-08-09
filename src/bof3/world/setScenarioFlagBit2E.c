#include "bof3/world/area02613_internal.h"

/* @behavior sets bit 0x2E in the active scenario flags.
 * @source 0x801F3338
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void setScenarioFlagBit2E(void) {
  func_8015B580((void*)D_8014686C, 0x2Eu);
}
