#include "bof3/bof3.h"

/* @source 0x801490C7 @kind data */
extern s8 D_801490C7;
/* @source 0x801490A8 @kind data */
extern u16 D_801490A8;
/* @source 0x801F294C @kind data */
extern u8 D_801F294C;
/* @source 0x801F294D @kind data */
extern u8 D_801F294D;

/* @source 0x801F3998
 * @behavior When the shared gate byte D_801490C7 is zero, stores 0xFFFF into
 * the shared status halfword D_801490A8 and clears D_801F294D. Otherwise stores
 * 0x5F into that halfword and increments D_801F294C modulo 256.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus5FAndIncrementByte801F294CIfGateElseFFFFAndClearByte801F294DWorld04Area17513_801F3998(void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0xFFFF;
    D_801F294D = 0;
  } else {
    u8 value = D_801F294C;

    D_801490A8 = 0x5F;
    D_801F294C = value + 1;
  }
}
