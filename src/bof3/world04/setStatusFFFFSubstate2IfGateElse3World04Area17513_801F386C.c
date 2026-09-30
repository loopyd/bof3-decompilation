#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_80146875;

/* @source 0x801F386C
 * @behavior Loads the shared gate byte D_801490C7, clears the shared status
 * halfword D_801490A8 to 0xFFFF, and then stores 2 into the shared sub-state
 * byte D_80146875 while the gate byte is nonzero, and 3 when the gate byte is
 * zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFSubstate2IfGateElse3World04Area17513_801F386C(void) {
  D_801490A8 = 0xFFFF;
  if (D_801490C7 != 0) {
    D_80146875 = 2;
  } else {
    D_80146875 = 3;
  }
}
