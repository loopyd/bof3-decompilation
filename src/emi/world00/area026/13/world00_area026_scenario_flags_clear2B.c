#include "internal.h"

/* @behavior clears bit 0x2B in the active scenario flags.
 * @source 0x801F3174
 */
void world00_area026_scenario_flags_clear2B(void) {
  func_8015B5A8((void*)D_8014686C, 0x2Bu);
}
