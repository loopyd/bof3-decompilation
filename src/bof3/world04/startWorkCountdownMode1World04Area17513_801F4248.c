#include "bof3/bof3.h"

extern u8 *D_1F800044;

/* @source 0x801F4248
 * @behavior Arms the scratchpad work countdown byte 0x0A with 0x20 and selects
 * handler mode 1 in the scratchpad work byte 0x04; both writes go through the
 * scratchpad work pointer cell at 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startWorkCountdownMode1World04Area17513_801F4248(void) {
  D_1F800044[0x0A] = 0x20;
  D_1F800044[0x04] = 1;
}
