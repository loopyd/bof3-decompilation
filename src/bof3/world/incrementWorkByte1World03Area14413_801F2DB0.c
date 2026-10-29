#include "bof3/bof3.h"

/* @source 0x801F2DB0
 * @behavior Increments the scratchpad work byte at pointer-slot 0x44 + 0x1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1World03Area14413_801F2DB0(void) {
  SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
