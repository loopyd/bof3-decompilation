#include "bof3/bof3.h"

/* @source 0x801F2E88
 * @behavior Increments the scratchpad work byte at pointer-slot 0x44 + 0x1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1World02Area08813_801F2E88(void) {
  SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
