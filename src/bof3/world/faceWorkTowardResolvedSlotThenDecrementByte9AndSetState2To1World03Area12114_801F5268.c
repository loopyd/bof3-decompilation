#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;
/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;

s32 faceSlotThenStepRotatingFacingUntilPositionHoldsNoRecordWorld03Area12114_801F5028(
    s32 x, s32 y, u8 slot);
s8 func_801BDE14(s32 x, s32 y, s32 range);
void func_801C24D0(void);
void func_801B0DFC(void);

/* @source 0x801F5268
 * @behavior Returns one only when the world state halfword 0x80143F00 is 0x79
 * and the range-1 record lookup at the work position words 0x34 and 0x38 of the
 * scratchpad pointer cell 0x1F800044 resolves a slot that the facing step of
 * 0x801F5028 reports as settled. On that path it runs the shared update at
 * 0x801C24D0, decrements work byte 0x09, runs 0x801B0DFC, writes 1 to byte
 * 0x12B of the record published at 0x80146250 and sets work byte 0x02 to 1.
 * Any other world state, or a facing step that reports zero, returns zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 faceWorkTowardResolvedSlotThenDecrementByte9AndSetState2To1World03Area12114_801F5268(void) {
  if (D_80143F00 != 0x79 ||
      (faceSlotThenStepRotatingFacingUntilPositionHoldsNoRecordWorld03Area12114_801F5028(
           *(s32*)(area12114WorkCursor_1F800044 + 0x34),
           *(s32*)(area12114WorkCursor_1F800044 + 0x38),
           func_801BDE14(*(s32*)(area12114WorkCursor_1F800044 + 0x34),
                         *(s32*)(area12114WorkCursor_1F800044 + 0x38), 1)) & 0xFF) == 0) {
    return 0;
  }
  func_801C24D0();
  area12114WorkCursor_1F800044[9] -= 1;
  func_801B0DFC();
  D_80146250[0x12B] = 1;
  area12114WorkCursor_1F800044[2] = 1;
  return 1;
}
