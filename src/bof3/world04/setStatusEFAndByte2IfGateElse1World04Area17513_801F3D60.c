#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801F294C;

/* @source 0x801F3D60
 * @behavior Loads the shared gate byte D_801490C7, stores 2 into the byte at
 * 0x801F294C while the gate byte is nonzero and 1 when the gate byte is zero,
 * then stores 0xEF into the shared status halfword D_801490A8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusEFAndByte2IfGateElse1World04Area17513_801F3D60(void) {
  s32 value;

  value = D_801490C7;
  if (value != 0) {
    value = 2;
  } else {
    value = 1;
  }
  D_801F294C = value;
  D_801490A8 = 0xEF;
}
