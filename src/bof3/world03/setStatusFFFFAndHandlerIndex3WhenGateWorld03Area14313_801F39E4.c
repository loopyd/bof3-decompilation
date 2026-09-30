#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F39E4
 * @behavior Loads the shared gate byte D_801490C7; while the gate byte is
 * nonzero it sets the handler index byte at 0x801D4286 to 3, and it then stores
 * 0xFFFF into the shared status halfword D_801490A8. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndHandlerIndex3WhenGateWorld03Area14313_801F39E4(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 3;
  }
  D_801490A8 = 0xFFFF;
}
