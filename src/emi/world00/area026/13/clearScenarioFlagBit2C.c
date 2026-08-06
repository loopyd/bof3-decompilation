#include "internal.h"

/* @behavior clears bit 0x2C in the active scenario flags.
 * @source 0x801F31C4
 */
void clearScenarioFlagBit2C(void) {
  func_8015B5A8((void*)D_8014686C, 0x2Cu);
}
