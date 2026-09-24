#include "bof3/bof3.h"

extern u16 D_80149328;
extern u8 D_80149333;

/* @source 0x801F51E4
 * @behavior Overlay setter: clears the halfword at 0x80149328 and writes the constant 0x02 to the byte at 0x80149333.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setTwoWorld03Area13413_801F51E4(void) {
  D_80149328 = 0;
  D_80149333 = 0x02;
}
