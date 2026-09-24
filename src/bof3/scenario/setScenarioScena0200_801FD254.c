#include "bof3/bof3.h"

extern u8 D_80146865;

/* @source 0x801FD254
 * @behavior Overlay setter: writes the constant 0x01 to the byte at 0x80146865.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioScena0200_801FD254(void) {
  D_80146865 = 0x01;
}
