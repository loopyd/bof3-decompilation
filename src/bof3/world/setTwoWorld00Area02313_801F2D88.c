#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F2D88
 * @behavior Overlay setter: clears the halfword at 0x8014932A and writes the constant 0x02 to the byte at 0x80149333.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setTwoWorld00Area02313_801F2D88(void) {
  D_8014932A = 0;
  D_80149333 = 0x02;
}
