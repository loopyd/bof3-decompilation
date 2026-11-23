#include "bof3/bof3.h"

/* Scratchpad pointer cell holding the work record of the current overlay step.
 * The original loads the cell three times, once before the state-byte test, once
 * inside the seeding block and once inside the drawing block, so each of those
 * sites names the cell. */
/* @source 0x1F800044 @kind data */
extern u8* D_1F800044;

/* The three coordinate runs of this overlay's own payload tail, which starts at
 * 0x801F3B34. Each run is addressed by the work byte at offset 0x0B alone, and
 * the runs sit four bytes apart: the byte runs hold the two position axes and
 * the run at 0x801F3B3C holds signed halfword entries at the doubled index.
 * Nothing outside this payload names them, so they stay address-anchored. */
/* @source 0x801F3B34 @kind data */
extern u8 D_801F3B34[];
/* @source 0x801F3B38 @kind data */
extern u8 D_801F3B38[];
/* @source 0x801F3B3C @kind data */
extern s16 D_801F3B3C[];

/* Shared overlay gate byte; bit 2 keeps this overlay's record drawer active. */
/* @source 0x8014832E @kind data */
extern u8 D_8014832E;

/* Shared focus coordinate pair of the main executable, the pair the window
 * comparison measures against. */
/* @source 0x8014930A @kind data */
extern s16 D_8014930A;
/* @source 0x8014930E @kind data */
extern s16 D_8014930E;

/* Record drawer of this overlay: it takes the work words at 0x34, 0x38 and 0x3C
 * as a vector, builds the rotation matrix around it and emits the record
 * primitives. */
/* @source 0x801F2D44 */
extern void func_801F2D44(void);

/* @source 0x801F2C04
 * @behavior Two independent steps. The seeding step, taken while the work state
 *           byte at offset 0x02 is zero, copies the low byte of the work
 *           halfword at offset 0x36 into work byte 0x0B, reloads the record from
 *           the scratchpad pointer cell, and writes the work vector triple from
 *           the three coordinate runs of this payload tail at that index: work
 *           word 0x34 becomes (run34[index] - 0x80) << 7 plus 0x38, work word
 *           0x38 becomes (run38[index] - 0x80) << 7 and work word 0x3C becomes
 *           the signed halfword run3C[index], after which it increments work
 *           byte 0x02 so the seeding step runs once. The window step always runs
 *           after it: while bit 2 of the shared gate byte 0x8014832E is set it
 *           reloads the record, reads that index once and calls the record
 *           drawer 0x801F2D44 only when the shared focus halfword 0x8014930A
 *           plus 6 and the shared focus halfword 0x8014930E plus 6 both lie
 *           within 11 of run34[index] and run38[index]. Every path returns
 *           through one epilogue. Takes no arguments and returns nothing; its
 *           0x18-byte frame holds $ra at 0x10 and the first pointer load
 *           precedes the frame creation. No word and no `jal` in this payload
 *           names 0x801F2C04, so the overlay loader is the only caller and the
 *           argument roles stay structural.
 *
 *           Order evidence: the seeding block falls through into the window
 *           block, so the two steps are sequential statements and a fresh seed
 *           is followed by the window test on the same tick. An else-if spelling
 *           needs a jump around the window block, and that jump is the first
 *           instruction the original does not carry.
 *
 *           Shape evidence: the window step is the same shared-focus gate family
 *           as advanceWorkByte2AndPlayCue200WhenCameraInsideRecordCoordinateWindowWorld04Area17013_801F3DE8,
 *           which fixes the shared-focus role of 0x8014930A and 0x8014930E, and
 *           the seeding step reads its own payload runs the way
 *           probeAxisPointRunAndSetFoundRecordOctantSideThenRequestAction2World03Area13913_801F308C
 *           reads its four-entry runs. A little-endian scan of every extracted
 *           payload under out/binaries finds this 0x140-byte span only at
 *           0x801F2C04 of this payload, and a masked scan of its leading
 *           0x28 bytes finds no second copy either, so no byte-identical twin
 *           exists and clone stayed ineligible. The range carries its own
 *           prologue and return, so the data and carve levers do not apply.
 *
 *           Lever evidence: the seeding statements keep the index read at each
 *           table store, which is why the original loads work byte 0x0B three
 *           times; a single index local collapses two of those loads and lands
 *           eight bytes short. The window comparison adds 6 to the focus value
 *           in its own statements, because one folded expression reassociates
 *           the constant onto the run byte and emits addiu on the wrong operand.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkVectorFromCoordinateRunsThenDrawWhenFocusNearPointWorld00Area03713_801F2C04(
    void) {
  u8* head;
  u8* work;
  u8 index;
  s32 dx;
  s32 dz;

  head = D_1F800044;
  if (FIELD_REF(u8, head, 0x02) == 0) {
    FIELD_REF(u8, head, 0x0B) = FIELD_REF(u16, head, 0x36);
    work = D_1F800044;
    FIELD_REF(s32, work, 0x34) =
        ((D_801F3B34[FIELD_REF(u8, work, 0x0B)] - 0x80) << 7) + 0x38;
    FIELD_REF(s32, work, 0x38) =
        (D_801F3B38[FIELD_REF(u8, work, 0x0B)] - 0x80) << 7;
    FIELD_REF(s32, work, 0x3C) = D_801F3B3C[FIELD_REF(u8, work, 0x0B)];
    FIELD_REF(u8, work, 0x02) = FIELD_REF(u8, work, 0x02) + 1;
  }
  if ((D_8014832E & 4) != 0) {
    index = D_1F800044[0x0B];
    dx = D_8014930A;
    dx += 6;
    dx -= D_801F3B34[index];
    if (__builtin_abs(dx) < 11) {
      dz = D_8014930E;
      dz += 6;
      dz -= D_801F3B38[index];
      if (__builtin_abs(dz) < 11) {
        func_801F2D44();
      }
    }
  }
}
