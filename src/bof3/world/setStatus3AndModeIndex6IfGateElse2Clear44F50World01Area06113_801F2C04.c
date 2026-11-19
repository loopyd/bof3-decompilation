#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;
extern s32 D_80144F50;

/* @source 0x801F2C04
 * @behavior Loads the shared signed gate byte D_801490C7. While that byte is
 * zero it clears the main-RAM word D_80144F50 and then stores 2 into the shared
 * status halfword D_801490A8; while the gate byte is nonzero it stores 3 into
 * D_801490A8 and then 6 into the shared mode index byte D_801D4285. Takes no
 * arguments and returns nothing. The 0x3 constant is materialised in $v0 and
 * fills the branch delay slot of the gate test, and the zeroed word is stored
 * before the status halfword.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus3AndModeIndex6IfGateElse2Clear44F50World01Area06113_801F2C04(void) {
  if (D_801490C7 == 0) {
    D_80144F50 = 0;
    D_801490A8 = 2;
  } else {
    D_801490A8 = 3;
    D_801D4285 = 6;
  }
}
