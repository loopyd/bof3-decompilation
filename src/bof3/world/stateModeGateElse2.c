#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2C04
 * @behavior Loads the shared signed gate byte D_801490C7. While that byte is
 * zero it stores 2 into the shared status halfword D_801490A8; while it is
 * nonzero it stores 4 into D_801490A8 and then 6 into the shared mode index
 * byte D_801D4285. Takes no arguments and returns nothing; the 0x4 constant is
 * materialised in $v0 and fills the branch delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateModeGateElse2(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 2;
  } else {
    D_801490A8 = 4;
    D_801D4285 = 6;
  }
}
