#include "bof3/ui/game00_internal.h"

/* @source 0x801A78F8
 * @behavior sets bit 7 in scenarioState.field_01
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void setScenarioField01Bit7(void) {
  scenarioState.field_01 |= 0x80;
}
