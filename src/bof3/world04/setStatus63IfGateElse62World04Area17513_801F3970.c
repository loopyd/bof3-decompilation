#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F3970
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x63 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0x62
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus63IfGateElse62World04Area17513_801F3970(void) {
  s32 value;

  value = D_801490C7;
  if (value != 0) {
    value = 0x63;
  } else {
    value = 0x62;
  }
  D_801490A8 = value;
}
