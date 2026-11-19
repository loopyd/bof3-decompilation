#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;

/* @source 0x801F2CF4
 * @behavior Loads the shared signed gate byte D_801490C7 and, while that byte
 * is nonzero, stores 4 into the shared status halfword D_801490A8 and then 6
 * into the shared mode index byte at 0x801D4285; when the gate byte is zero it
 * stores 0xFFFF into D_801490A8 instead and touches nothing else. Takes no
 * arguments and returns nothing; leaf with no frame, the 0x4 status constant is
 * materialised in $v0 and fills the branch delay slot, and the 0xFFFF fallback
 * is built with ori. Every word of this span equals the span at 0x801F2CB0 in
 * this payload except the jump immediate, which differs by the 0x44 span delta
 * between the two entry points.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus4AndModeIndex6IfGateElseFFFFWorld01Area06113_801F2CF4(void) {
  if (D_801490C7 != 0) {
    D_801490A8 = 4;
    D_801D4285 = 6;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
