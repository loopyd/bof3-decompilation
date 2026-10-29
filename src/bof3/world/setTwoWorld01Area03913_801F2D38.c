#include "bof3/bof3.h"

extern u16 D_8014932C;
extern u8 D_80149333;

/* @source 0x801F2D38
 * @behavior Overlay setter: clears the halfword at 0x8014932C and writes the constant 0x02 to the byte at 0x80149333.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setTwoWorld01Area03913_801F2D38(void) {
  D_8014932C = 0;
  D_80149333 = 0x02;
}
