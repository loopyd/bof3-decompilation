#include "bof3/bof3.h"

/* @source 0x80145AA4 @kind unknown */
extern u16 D_80145AA4;

/* @source 0x801F5684
 * @behavior Tests halfword 0x80145AA4 in priority order: bit 0x4000
 * toggles bit 2 of work byte 8, bit 0x2000 increments it, and bit 0x8000
 * decrements it. These paths and bit 0x1000 mask the byte to 0..7 and
 * return one; otherwise returns zero without writing. The work pointer
 * comes from scratchpad slot 0x44. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 updateWorkByte8FromMaskWorld03Area12114_801F5684(void) {
  u16 mask;
  u8* work;

  mask = D_80145AA4;
  if (mask & 0x4000) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[8] ^= 4;
  } else if (mask & 0x2000) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[8]++;
  } else if (mask & 0x8000) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[8]--;
  } else if (!(mask & 0x1000)) {
    return 0;
  }
  SPAD_PTR_SLOT(u8, 0x44)[8] &= 7;
  return 1;
}
