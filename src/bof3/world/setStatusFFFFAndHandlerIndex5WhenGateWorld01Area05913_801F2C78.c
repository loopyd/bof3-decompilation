#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4286;

/* @source 0x801F2C78
 * @behavior Loads the shared gate byte D_801490C7; while that byte is nonzero
 * the handler index byte at 0x801D4286 is set to 5. The shared status halfword
 * D_801490A8 is then stored with 0xFFFF on every path. Takes no arguments and
 * returns nothing; the 0x5 constant is materialised in $v0 and fills the
 * branch delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndHandlerIndex5WhenGateWorld01Area05913_801F2C78(void) {
  if (D_801490C7 != 0) {
    D_801D4286 = 5;
  }
  D_801490A8 = 0xFFFF;
}
