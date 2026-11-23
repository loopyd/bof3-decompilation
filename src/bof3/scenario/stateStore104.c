#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;

/* @source 0x801FD934
 * @behavior Overlay setter: writes the constant 0x68 to the byte at 0x80146864.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStore104(void) {
  g_ScenarioProgress = 0x68;
}
