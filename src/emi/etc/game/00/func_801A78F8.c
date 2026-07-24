#include "internal.h"

/* @source 0x801A78F8
 * @behavior sets bit 7 in GAME_SCENARIO_STATE.field_01
 */
void func_801A78F8(void) {
  GAME_SCENARIO_STATE.field_01 |= 0x80;
}
