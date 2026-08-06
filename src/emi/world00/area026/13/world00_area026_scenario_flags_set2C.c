#include "internal.h"

/* @behavior sets bit 0x2C in the active scenario flags.
 * @source 0x801F319C
 */
void world00_area026_scenario_flags_set2C(void) {
  func_8015B580((void*)D_8014686C, 0x2Cu);
}
