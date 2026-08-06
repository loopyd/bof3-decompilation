#include "internal.h"

/* @behavior clears bit 0x2C in the active scenario flags.
 * @source 0x801F31C4
 */
void world00_area026_scenario_flags_clear2C(void) {
  func_8015B5A8((void*)D_8014686C, 0x2Cu);
}
