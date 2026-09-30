#include "bof3/bof3.h"

extern u8 D_8014832E;
extern s8 D_80146872;

/* @source 0x801F72C4
 * @behavior Handler reached from the SCENA11 overlay dispatch table: arms the shared effect bank D_8014832E with 0x1F and then advances the signed main-RAM state byte D_80146872 to 1, which selects the next entry of that same dispatch table. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectBankAndAdvanceStateScenarioScena1100_801F72C4(void) {
  D_8014832E = 0x1F;
  D_80146872 = 1;
}
