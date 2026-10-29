#include "bof3/bof3.h"

extern u8 D_801448EB;

/* @source 0x801F2F08
 * @behavior Overlay setter: writes the constant 0x22 to the byte at 0x801448EB.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorld03Area13113_801F2F08(void) {
  D_801448EB = 0x22;
}
