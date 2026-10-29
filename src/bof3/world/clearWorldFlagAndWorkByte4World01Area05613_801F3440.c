#include "bof3/bof3.h"

extern u16 D_8014625A;

/* @source 0x801F3440
 * @behavior Reads the scratchpad work pointer from slot 0x44, clears bit 0x0200
 * of the world-flags halfword D_8014625A, and zeroes work byte 4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorldFlagAndWorkByte4World01Area05613_801F3440(void) {
  u8 *work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  D_8014625A &= 0xFDFF;
  work[4] = 0;
}
