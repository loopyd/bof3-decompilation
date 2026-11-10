#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2D10
 * @behavior Loads the shared signed gate byte D_801490C7. When nonzero,
 * stores 4 into status halfword D_801490A8 and 6 into mode byte D_801D4285;
 * otherwise stores 0xFFFF into the status halfword without changing the mode.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus4AndModeIndex6IfGateElseFFFFWorld03Area11613_801F2D10(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 4;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
