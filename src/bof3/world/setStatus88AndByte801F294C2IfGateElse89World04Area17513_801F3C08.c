#include "bof3/bof3.h"

/* @source 0x801490C7 @kind data */
extern s8 D_801490C7;
/* @source 0x801490A8 @kind data */
extern u16 D_801490A8;
/* @source 0x801F294C @kind data */
extern u8 D_801F294C;

/* @source 0x801F3C08
 * @behavior Reads the shared gate byte D_801490C7. When it is zero, stores
 * 0x89 into the shared status halfword D_801490A8. Otherwise stores 0x88
 * into that halfword and 2 into the byte at 0x801F294C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus88AndByte801F294C2IfGateElse89World04Area17513_801F3C08(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0x89;
  } else {
    D_801490A8 = 0x88;
    D_801F294C = 2;
  }
}
