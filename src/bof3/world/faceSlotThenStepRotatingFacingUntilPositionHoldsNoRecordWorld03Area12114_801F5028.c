#include "bof3/bof3.h"

s32 classifyDirectionToPointWorld03Area12114_801F4F78(s32 x, s32 y, u8 slot);
s32 func_801B0E44(void);
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* @source 0x801F5028
 * @behavior Faces the work slot named by the byte index against the point,
 * storing the classified direction into facing byte 0x08 of the work record
 * published at the scratchpad pointer cell 0x1F800044. Up to eight times it
 * runs the no-argument step at 0x801B0E44 and, when that step reports zero in
 * its low byte, asks 0x801BDE14 for a work record within range zero of the
 * same point. A step that reports nonzero, or a search that names a record,
 * rotates the facing byte by one modulo eight and repeats; a search that
 * returns -1 stops. A slot of 0xFF returns zero without touching memory.
 * Returns one when a search ended the loop before the eighth rotation and
 * zero when all eight facings were rotated or the slot was 0xFF.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 faceSlotThenStepRotatingFacingUntilPositionHoldsNoRecordWorld03Area12114_801F5028(
    s32 x, s32 y, u8 slot) {
  u8* work;
  s32 i;

  if (slot != 0xFF) {
    SPAD_PTR_SLOT(u8, 0x44)[8] =
        classifyDirectionToPointWorld03Area12114_801F4F78(x, y, slot);
    for (i = 0; i < 8; i++) {
      if ((func_801B0E44() & 0xFF) == 0) {
        if (func_801BDE14(x, y, 0) == -1) {
          break;
        }
      }
      work = SPAD_PTR_SLOT(u8, 0x44);
      work[8] = (work[8] + 1) & 7;
    }
    if (i < 8) {
      return 1;
    }
  }
  return 0;
}
