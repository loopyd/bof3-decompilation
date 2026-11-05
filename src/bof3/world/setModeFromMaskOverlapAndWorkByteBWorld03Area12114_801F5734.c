#include "bof3/bof3.h"

/* @source 0x80145AA4 @kind unknown */
extern u16 D_80145AA4;
/* @source 0x80145AB6 @kind unknown */
extern u16 D_80145AB6;
/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;

/* @source 0x801F5734
 * @behavior Sets byte 0x11C of the record at 0x80146250 to 4 when the
 * halfwords at 0x80145AA4 and 0x80145AB6 overlap and byte 0x0B of the
 * scratchpad slot 0x44 work record is below 64; otherwise sets it to 3.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setModeFromMaskOverlapAndWorkByteBWorld03Area12114_801F5734(void) {
  if ((D_80145AA4 & D_80145AB6) != 0 &&
      SPAD_PTR_SLOT(u8, 0x44)[0x0B] < 64) {
    D_80146250[0x11C] = 4;
  } else {
    D_80146250[0x11C] = 3;
  }
}
