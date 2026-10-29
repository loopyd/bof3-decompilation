#include "bof3/bof3.h"

extern u8* D_80146250;
extern u16 D_8014932E;
extern u16 D_80149330;

/* @source 0x801F564C
 * @behavior Overlay state reset: stores 3 into the mode byte at offset 0x11C of
 * the shared work record at 0x80146250, clears the shared halfwords at
 * 0x8014932E and 0x80149330, then clears the three 32-bit words at offsets
 * 0x0C/0x10/0x14 of the work record published at the scratchpad pointer slot
 * 0x44. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetMode3AndWorkStateWorld03Area12114_801F564C(void) {
  u32* record;

  D_80146250[0x11C] = 3;
  record = SPAD_PTR_SLOT(u32, 0x44);
  D_8014932E = 0;
  D_80149330 = 0;
  record[3] = 0;
  record[4] = 0;
  record[5] = 0;
}
