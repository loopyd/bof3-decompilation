#include "bof3/bof3.h"

/* @source 0x801490C7 @kind data */
extern s8 D_801490C7;
/* @source 0x801490A8 @kind data */
extern u16 D_801490A8;
/* @source 0x801F294D @kind data */
extern u8 D_801F294D;

/* @source 0x801F3A84
 * @behavior Reads the shared gate byte D_801490C7. When it is zero, stores
 * 0xFFFF into the shared status halfword D_801490A8. Otherwise stores 0x77
 * into that halfword and 0x0A into the byte at 0x801F294D.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus77AndByte801F294D0AIfGateElseFFFFWorld04Area17513_801F3A84(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0xFFFF;
  } else {
    D_801490A8 = 0x77;
    D_801F294D = 0x0A;
  }
}
