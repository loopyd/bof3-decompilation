#include "internal.h"

/* @behavior sets bit 0x29, then clears bit 0x28 in the active scenario flags.
 * @source 0x801F309C
 */
void setScenarioFlagBit29ClearBit28(void) {
  func_8015B580((void*)D_8014686C, 0x29u);
  func_8015B5A8((void*)D_8014686C, 0x28u);
}
