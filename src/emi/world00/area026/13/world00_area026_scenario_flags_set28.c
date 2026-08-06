#include "internal.h"

/* @behavior sets bit 0x28 in the active scenario flags.
 * @source 0x801F30FC
 */
void world00_area026_scenario_flags_set28(void) {
  func_8015B580((void*)D_8014686C, 0x28u);
}
