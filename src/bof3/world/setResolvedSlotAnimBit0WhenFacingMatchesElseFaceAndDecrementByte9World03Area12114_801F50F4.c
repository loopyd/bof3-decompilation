#include "bof3/bof3.h"

/* Scratchpad pointer cell 0x1F800044 holding the active work record. The cell is
 * a variable rather than a constant address so that each use reloads it, as the
 * original does before the record route read, before the slot facing test and
 * before each of the two state stores.
 * @source 0x1F800044 @kind unknown
 */
extern u8* area12114WorkCursor_1F800044;

/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;

/* Overlapping pad halfwords of the shared main executable. The body of this
 * function only tests whether the two halfwords share a set bit; no bit of
 * either halfword is named here.
 * @source 0x80145AA4 @kind unknown
 */
extern u16 D_80145AA4;
/* @source 0x80145AB6 @kind unknown */
extern u16 D_80145AB6;

/* Record pointer cell of the shared main executable whose byte 0x12B the
 * settled path raises. Its role beyond that byte is not established.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Route offset tables of the shared main executable: eight-byte records holding
 * the first-coordinate offset at 0x80181B94 and the second-coordinate offset at
 * 0x80181B98, selected by the active work area route byte at offset 0x08. The
 * original scales that byte by eight once, which is element index route * 2, and
 * doubles the loaded word before it adds it to the matching record word, so both
 * bases stay their own symbol.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Slot view of the shared work-record array: the byte at 0x80146890 is field
 * 0x08 of the 0x98-byte work record, the route byte the facing tables are
 * indexed by, and the byte at 0x801468FC is the low byte of the record
 * animation halfword 0x74, whose bit 0 this function raises. Only those two
 * bytes are named.
 */
typedef struct Area12114SlotView {
  u8 route_00;
  u8 unk_01[0x6B];
  u8 anim_6C;
  u8 unk_6D[0x2B];
} Area12114SlotView;

/* @source 0x80146890 @kind data */
extern Area12114SlotView D_80146890[];

s32 faceSlotThenStepRotatingFacingUntilPositionHoldsNoRecordWorld03Area12114_801F5028(
    s32 x, s32 y, u8 slot);
s8 func_801BDE14(s32 x, s32 y, s32 range);
void func_801C24D0(void);
void func_801B0DFC(void);

/* @source 0x801F50F4
 * @behavior Requires the world state halfword 0x80143F00 to be 0x79 and then
 * projects the active work record: the position words 0x34 and 0x38 of the
 * scratchpad pointer cell 0x1F800044 are offset by the doubled route words at
 * 0x80181B94 and 0x80181B98 selected by the record route byte 0x08. It asks
 * 0x801BDE14 for a range-1 record at that point and, when the pad halfwords
 * 0x80145AA4 and 0x80145AB6 overlap and the active route byte equals the low
 * three bits of the resolved slot route byte at 0x80146890, raises bit 0 of
 * that slot animation byte at 0x801468FC and returns without moving. Otherwise,
 * when the facing step of 0x801F5028 reports the slot settled, it runs the
 * shared update at 0x801C24D0, decrements work byte 0x09, runs 0x801B0DFC,
 * writes 1 to byte 0x12B of the record published at 0x80146250 and sets work
 * byte 0x02 to 1. A facing step that reports zero, an unset world state, or a
 * matched facing returns with no further store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setResolvedSlotAnimBit0WhenFacingMatchesElseFaceAndDecrementByte9World03Area12114_801F50F4(
    void) {
  u8* work;
  u8 route;
  s32 x;
  s32 y;
  s8 slot;

  if (D_80143F00 != 0x79) {
    return;
  }
  work = area12114WorkCursor_1F800044;
  route = work[8];
  x = *(s32*)(work + 0x34) + D_80181B94[route * 2] * 2;
  y = *(s32*)(work + 0x38) + D_80181B98[route * 2] * 2;
  slot = func_801BDE14(x, y, 1);
  if ((D_80145AA4 & D_80145AB6) != 0 &&
      area12114WorkCursor_1F800044[8] == (D_80146890[(u8)slot].route_00 & 7)) {
    D_80146890[(u8)slot].anim_6C |= 1;
    return;
  }
  if ((faceSlotThenStepRotatingFacingUntilPositionHoldsNoRecordWorld03Area12114_801F5028(
           x, y, (u8)slot) &
       0xFF) == 0) {
    return;
  }
  func_801C24D0();
  area12114WorkCursor_1F800044[9] -= 1;
  func_801B0DFC();
  D_80146250[0x12B] = 1;
  area12114WorkCursor_1F800044[2] = 1;
}
