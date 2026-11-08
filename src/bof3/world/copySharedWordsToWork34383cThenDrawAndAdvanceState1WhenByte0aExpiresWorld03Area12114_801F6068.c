#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x80145ECC @kind unknown */
extern u32 D_80145ECC;

void advanceWorkHalfword3eBy10AndSetState3At0x281ThenDrawRotatedTexturedDiamondWorld03Area12114_801F60F4(
    void);

/* @source 0x801F6068
 * @behavior Byte-1 state handler 3 of the byte-1 handler table 0x801F68A4 for
 * the work record published at the scratchpad pointer cell 0x1F800044. While
 * the countdown byte 0x0A is nonzero it decrements that byte and returns. When
 * the countdown has expired it copies the shared position words 0x80145EC4 and
 * 0x80145EC8 into work offsets 0x34 and 0x38 and the word 0x80145ECC plus
 * 0x800000 into offset 0x3C, runs the overlay draw step at 0x801F60F4, then
 * increments work byte 0x01 through a second read of the pointer cell so the
 * next frame dispatches to that draw step directly. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySharedWordsToWork34383cThenDrawAndAdvanceState1WhenByte0aExpiresWorld03Area12114_801F6068(
    void) {
  u8* work;
  u8* cursor;
  u8 countdown;

  work = area12114WorkCursor_1F800044;
  countdown = work[0x0A];
  if (countdown == 0) {
    FIELD_REF(u32, work, 0x34) = D_80145EC4;
    FIELD_REF(u32, work, 0x38) = D_80145EC8;
    FIELD_REF(u32, work, 0x3C) = D_80145ECC + 0x800000u;
    advanceWorkHalfword3eBy10AndSetState3At0x281ThenDrawRotatedTexturedDiamondWorld03Area12114_801F60F4();
    cursor = area12114WorkCursor_1F800044;
    cursor[1] += 1;
  } else {
    work[0x0A] = countdown - 1;
  }
}
