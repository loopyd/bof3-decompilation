#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F3A34
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x70 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xFFFF
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus70IfGateElseFFFFWorld04Area17513_801F3A34(void) {
  s32 value;

  value = D_801490C7;
  if (value != 0) {
    value = 0x70;
  } else {
    value = 0xFFFF;
  }
  D_801490A8 = value;
}
