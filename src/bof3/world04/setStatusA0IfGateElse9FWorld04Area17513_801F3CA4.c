#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F3CA4
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xA0 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0x9F
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusA0IfGateElse9FWorld04Area17513_801F3CA4(void) {
  s32 value;

  value = D_801490C7;
  if (value != 0) {
    value = 0xA0;
  } else {
    value = 0x9F;
  }
  D_801490A8 = value;
}
