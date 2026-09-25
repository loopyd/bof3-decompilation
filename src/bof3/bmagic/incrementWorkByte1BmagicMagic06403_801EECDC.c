#include "bof3/bof3.h"

/* @source 0x801EECDC
 * @behavior Increments the byte at offset 1 of the scratchpad pointer held in slot 0x44.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1BmagicMagic06403_801EECDC(void) {
  SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
