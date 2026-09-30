#include "bof3/bof3.h"

extern s16 D_8014932C;
extern u8 D_80149333;

/* @source 0x801F3E00
 * @behavior Overlay setter: stores the signed halfword -0x100 (0xFF00) into
 * 0x8014932C and then the constant 0x02 into the byte at 0x80149333. Takes no
 * arguments and returns nothing; the immediates reach both stores through $v0
 * and no frame is built.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setHalfwordFF00AndTwoWorld01Area04113_801F3E00(void) {
  D_8014932C = -0x100;
  D_80149333 = 0x02;
}
