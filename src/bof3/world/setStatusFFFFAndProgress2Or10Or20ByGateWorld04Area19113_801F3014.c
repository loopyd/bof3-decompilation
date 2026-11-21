#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_801448EC; /* @source 0x801448EC @kind unknown */

/* @source 0x801F3014
 * @behavior Entry 4 of the handler pointer run the payload holds at 0x801F4168:
 * loads the shared gate byte D_801490C7 into a local, stores 0xFFFF into the
 * shared status halfword D_801490A8, and then stores into the shared front-end
 * mode byte D_801448EC the value 2 while the gate byte is zero, 10 while it is
 * one, and 20 for every other value. The status store precedes the gate test and
 * the gate byte is read once, so the selection merges into a single `sb` at the
 * join. Takes no arguments, calls nothing and returns nothing; leaf body with no
 * frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndProgress2Or10Or20ByGateWorld04Area19113_801F3014(void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFFu;
  if (gate == 0) {
    D_801448EC = 2;
  } else if (gate == 1) {
    D_801448EC = 10;
  } else {
    D_801448EC = 20;
  }
}
