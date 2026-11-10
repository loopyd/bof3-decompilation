#include "bof3/bof3.h"

/* @source 0x801490C7 @kind data */
extern s8 D_801490C7;
/* @source 0x801490A8 @kind data */
extern u16 D_801490A8;
/* @source 0x801F294C @kind data */
extern u8 D_801F294C;

/* @source 0x801F3DDC
 * @behavior Reads the shared gate byte D_801490C7. When it is zero, stores
 * 0xFFFF into the shared status halfword D_801490A8 and clears the byte at
 * 0x801F294C. Otherwise stores 0xF7 into that halfword and 4 into that byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusF7AndByte801F294C4IfGateElseFFFFAndByte0World04Area17513_801F3DDC(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0xFFFF;
    D_801F294C = 0;
  } else {
    D_801490A8 = 0xF7;
    D_801F294C = 4;
  }
}
