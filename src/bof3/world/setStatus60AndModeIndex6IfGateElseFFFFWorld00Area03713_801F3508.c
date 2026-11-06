#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F3508
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 * nonzero, stores 0x60 into the shared status halfword D_801490A8 and 6 into the
 * shared mode byte at 0x801D4285; when the gate byte is zero it stores 0xFFFF
 * into D_801490A8 instead and touches nothing else. Takes no arguments and
 * returns nothing; leaf with no frame, and the 0x60 constant is materialised in
 * $v0 and fills the branch delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus60AndModeIndex6IfGateElseFFFFWorld00Area03713_801F3508(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 0x60;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
