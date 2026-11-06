#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F3960
 * @behavior Loads the shared gate byte D_801490C7; while the gate byte is
 * nonzero it sets the handler index byte at 0x801D4286 to 5, and it then stores
 * 0xFFFF into the shared status halfword D_801490A8. Takes no arguments and
 * returns nothing.
 * Constant/variance evidence: the 48 bytes 0x801F3960..0x801F3990 are
 * byte-identical to the already-exact closure
 * setStatusFFFFAndHandlerIndex5WhenGateWorld03Area14313_801F3A14, and differ
 * from the same-target sibling at 0x801F3930 only in that stored index
 * (3 vs 5).
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndHandlerIndex5WhenGateWorld01Area06813_801F3960(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 5;
  }
  D_801490A8 = 0xFFFF;
}
