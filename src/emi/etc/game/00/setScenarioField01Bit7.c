#include "internal.h"

/* @source 0x801A78F8
 * @behavior sets bit 7 in scenarioState.field_01
 */
void setScenarioField01Bit7(void) {
  scenarioState.field_01 |= 0x80;
}
