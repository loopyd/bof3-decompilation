#include "internal.h"

/* @behavior clears bit 0x29 in the active scenario flags.
 * @source 0x801F30D4
 */
void clearScenarioFlagBit29(void) {
  func_8015B5A8((void*)D_8014686C, 0x29u);
}
