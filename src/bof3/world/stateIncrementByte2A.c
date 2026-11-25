#include "bof3/bof3.h"

/* @source 0x801F403C
 * @behavior Increments the scratchpad work byte at pointer-slot 0x44 + 0x2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateIncrementByte2A(void) {
  SPAD_PTR_SLOT(u8, 0x44)[2]++;
}
