#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern s8 D_801448EC;

/* @source 0x801F392C
 * @behavior Loads the shared gate byte D_801490C7 and, while it equals 1,
 * stores 0x5F into the shared status halfword D_801490A8 and 1 into the shared
 * byte at 0x801448EC; when the gate byte differs from 1 it stores 0xFFFF into
 * D_801490A8 instead and touches nothing else.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus5FAndByte801448ECIfGateElseFFFFWorld04Area17513_801F392C(void) {
  if (D_801490C7 == 1) {
    D_801490A8 = 0x5F;
    D_801448EC = 1;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
