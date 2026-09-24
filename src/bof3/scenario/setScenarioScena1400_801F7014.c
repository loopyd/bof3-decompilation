#include "bof3/bof3.h"

extern u8 D_80146872;

/* @source 0x801F7014
 * @behavior Overlay setter: writes the constant 0x01 to the byte at 0x80146872.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioScena1400_801F7014(void) {
  D_80146872 = 0x01;
}
