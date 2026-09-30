#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F3C4C
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x8A into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xFFFF
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus8AIfGateElseFFFFWorld04Area17513_801F3C4C(void) {
  s32 value;

  value = D_801490C7;
  if (value != 0) {
    value = 0x8A;
  } else {
    value = 0xFFFF;
  }
  D_801490A8 = value;
}
