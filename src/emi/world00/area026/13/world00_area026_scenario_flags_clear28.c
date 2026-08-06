#include "internal.h"

/* @behavior clears bit 0x28 in the active scenario flags.
 * @source 0x801F3124
 */
void world00_area026_scenario_flags_clear28(void) {
  func_8015B5A8((void*)D_8014686C, 0x28u);
}
