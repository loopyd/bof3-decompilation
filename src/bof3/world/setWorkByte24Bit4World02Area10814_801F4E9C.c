#include "bof3/bof3.h"

/* @source 0x801F4E9C
 * @behavior Sets bit 4 of the scratchpad work byte at pointer-slot 0x44 + 0x24.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte24Bit4World02Area10814_801F4E9C(void) {
  SPAD_PTR_SLOT(u8, 0x44)[0x24] |= 0x10u;
}
