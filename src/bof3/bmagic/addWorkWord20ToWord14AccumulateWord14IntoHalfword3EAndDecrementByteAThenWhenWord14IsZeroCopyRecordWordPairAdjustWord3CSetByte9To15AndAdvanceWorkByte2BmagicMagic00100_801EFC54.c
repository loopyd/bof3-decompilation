#include "bof3/bof3.h"

/* Record pointer published by the main executable, outside this overlay's
 * image. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, where the remaining asm spans func_801EF1CC and
 * func_801F014C still reference it, so this lift needs no target-map row. The
 * offsets read here are the ones the exact bmagic siblings copy from the record
 * they read through 0x8014638C, for example
 * copyWorkWordTripletSetBytes9To4AndAToB0ThenAdvanceWorkByte2BmagicMagic07903_801EFC74
 * and copyWorkWordPairAndHalfwordClearBytes9AThenAdvanceWorkByte2BmagicMagic10303_801EEF94.
 * The pointer cell itself is declared volatile: the original re-reads it before
 * each of the three uses, including twice across a store through the work
 * object, which the volatile cell reproduces. The exact sibling
 * addWorkMotionPairAndIncrementBytes9AThenAdvanceWorkByte2At10BmagicMagic04303_801EF55C
 * uses the same volatile cell spelling for the same reload pattern. */
extern u8 *volatile D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle work pointer in main RAM. The record layout is shared with the
 * sibling lifts of this target, which read its words 0x34 and 0x38 through this
 * same pointer declaration. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, the form this target's other lifts use
 * (subtractWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2WhenChannelIs80BmagicMagic00100_801EF5CC).
 * The original re-reads the cell after a store through the previously loaded
 * pointer, because that store may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EFC54
 * @behavior Per-frame field integrator for the scratchpad work object. It reads
 *           the pointer cell at 0x1F800044 once, adds work word 0x20 into work
 *           word 0x14, adds the new word 0x14 into the halfword at 0x3E, and
 *           decrements work byte 0xA while it is nonzero. It then re-reads the
 *           cell and returns while the new word 0x14 is still nonzero; when
 *           word 0x14 has reached zero it copies words 0x34 and 0x38 of the
 *           record published at 0x8014638C into the same offsets of the work
 *           object, sets work byte 9 to 0xF, subtracts from work word 0x3C the
 *           difference between word 0x3C of the record published at 0x801EB4E0
 *           and word 0x3C of the 0x8014638C record, and advances work byte 2,
 *           the handler index, with a byte-width read-modify-write. The
 *           0x8014638C pointer cell is read three times, once for each record
 *           offset, because the original re-reads it across its stores through
 *           the work object; the 0x3C difference is held in a local whose two
 *           loads precede the byte 9 store, and the work cell is read again
 *           before the handler advance, which the original schedules ahead of
 *           the 0x3C store. Leaf with no frame;
 *           takes no arguments, returns nothing and reads or writes no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkWord20ToWord14AccumulateWord14IntoHalfword3EAndDecrementByteAThenWhenWord14IsZeroCopyRecordWordPairAdjustWord3CSetByte9To15AndAdvanceWorkByte2BmagicMagic00100_801EFC54(void) {
  u8 *work;
  u8 countdown;
  u8 *copy;
  u8 *source;
  u8 *record;
  u8 *origin;
  u32 delta;
  u8 *step;

  work = D_1F800044;
  *(u32 *)(work + 0x14) += *(u32 *)(work + 0x20);
  countdown = work[0xA];
  *(u16 *)(work + 0x3E) += *(u32 *)(work + 0x14);
  if (countdown != 0) {
    work[0xA] = countdown - 1;
  }
  copy = D_1F800044;
  if (*(u32 *)(copy + 0x14) != 0) {
    return;
  }
  source = D_8014638C;
  *(u32 *)(copy + 0x34) = *(u32 *)(source + 0x34);
  source = D_8014638C;
  *(u32 *)(copy + 0x38) = *(u32 *)(source + 0x38);
  record = D_801EB4E0;
  origin = D_8014638C;
  delta = *(u32 *)(record + 0x3C) - *(u32 *)(origin + 0x3C);
  copy[9] = 0xF;
  step = D_1F800044;
  *(u32 *)(copy + 0x3C) -= delta;
  step[2]++;
}
