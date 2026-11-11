#include "bof3/bof3.h"

/* Scratchpad pointer cell holding this overlay's work record, which is also
 * the point pair the position probe reads: the original loads the cell at the
 * head of each probe iteration and a second time before the octant store, so
 * each of those sites names the cell. */
/* @source 0x1F800044 @kind bss */
extern u8* D_1F800044;

/* The two four-entry probe coordinate runs of this overlay's own payload tail,
 * both interior bytes of the reviewed run at 0x801F442C. Mode 0 pairs each
 * entry of the run at 0x801F4440 with the fixed word 0x5C0000 and mode 1 pairs
 * each entry of the run at 0x801F4444 with 0x440000, so one run supplies the
 * axis the other fixes. Nothing outside this payload names them, so they stay
 * address-anchored. */
/* @source 0x801F4440 @kind data */
extern u8 D_801F4440[];
/* @source 0x801F4444 @kind data */
extern u8 D_801F4444[];

/* Octant byte and the two position words of the 0x140-byte-stride work record
 * array of the main executable, all indexed here by the record number the
 * position probe answers. The byte at 0x80145E98 is the first byte of the
 * stride and the words at 0x80145EC4 and 0x80145EC8 are the record position
 * pair the probed point is compared against. All three lie outside this
 * payload in the shared main image, so they keep the names the World03/Area148
 * record update of 0x801F4614 gives the same cells. */
/* @source 0x80145E98 @kind table */
extern u8 D_80145E98[];
/* @source 0x80145EC4 @kind data */
extern s32 D_80145EC4[];
/* @source 0x80145EC8 @kind data */
extern s32 D_80145EC8[];

/* World flag halfword and front-end phase byte of the shared main image; the
 * original skips a probed record while either gate is closed. */
/* @source 0x8014625A @kind data */
extern u16 D_8014625A;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Position probe of the main executable: it answers the record number that
 * occupies the supplied 16.16 point pair, or 0xFF when that point is free. */
/* @source 0x801BE0C0 */
extern s32 func_801BE0C0(s32 arg0, s32 arg1, u8 arg2);
/* Action request of the main executable, called with action 2 here. */
/* @source 0x8015C214 */
extern void func_8015C214(u8 arg0, s32 arg1);

/* @source 0x801F308C
 * @behavior Probe-run seeding of this overlay, called with the axis to scan:
 *           argument 0 runs the four points that fix x at 0x5C0000 and take y
 *           from the byte run at 0x801F4440, argument 1 runs the four points
 *           that fix y at 0x440000 and take x from the byte run at 0x801F4444,
 *           and any other argument returns at once. Each iteration writes the
 *           16.16 point pair into the work record published at the scratchpad
 *           pointer cell 0x1F800044, mode 0 fixing the word at offset 0x34 at
 *           0x5C0000 and filling offset 0x38 from the run while mode 1 does the
 *           reverse, re-reads the word at 0x34 and passes the pair with a zero
 *           third argument to the shared position probe at 0x801BE0C0, which
 *           answers the number of the record occupying that point or 0xFF when
 *           the point is free. A probed point is skipped when the answer is
 *           0xFF, when the shared flag halfword 0x8014625A holds the 0x1400
 *           mask, or when the front-end phase byte 0x80143BB0 is non-zero.
 *           Otherwise the answer selects the answered record of the
 *           0x140-byte-stride array whose first byte is 0x80145E98, and that
 *           record's octant byte is set from the order of the probed axis word
 *           against the record's own position word: mode 0 compares the
 *           0x5C0000 just stored (re-read from offset 0x34) against the record
 *           word at 0x80145EC4 and writes 3 on the greater answer and 7
 *           otherwise, while mode 1 compares the 0x440000 just stored
 *           (re-read from offset 0x38) against the record word at 0x80145EC8
 *           and writes 5 on the greater answer and 1 otherwise. Action 2 is
 *           then requested for the answered record through 0x8015C214. Both
 *           runs scan four points. Takes the axis byte and returns nothing; the
 *           0x20-byte frame keeps $ra at 0x18 and $s1 and $s0 at 0x14 and 0x10.
 *
 *           Name evidence: the two four-entry runs and the four stored octant
 *           values are read from this payload's own bytes, and the roles of
 *           0x801BE0C0, 0x8015C214, 0x8014625A, 0x80143BB0, 0x80145E98,
 *           0x80145EC4 and 0x80145EC8 are the ones the World03/Area148 record
 *           update at 0x801F4614 and the World03/Area139 byte-2 handlers of
 *           this payload give the same shared cells.
 *
 *           Shape evidence: the probe call with the record word at 0x34 first,
 *           the 0xFF answer test, the 0x1400 flag halfword test and the
 *           front-end phase byte test, the 0x140-byte stride built as
 *           (answer << 2) + answer shifted left by six, and the closing
 *           func_8015C214(answer, 2) are the statement order of that
 *           World03/Area148 update, which supplied the pointer-local spelling
 *           and the reload points; this body differs by carrying no saved
 *           +0x3C word, by seeding the octant instead of stepping it, and by
 *           taking its probe point from one fixed word and one table byte. A
 *           little-endian masked scan of the 395 payloads under out/binaries
 *           with the 25 lui, j and jal words masked finds this 0x1FC-byte body
 *           nowhere, so no byte-identical sibling exists and clone was
 *           declined. The scan index is cleared ahead of the run base, which
 *           is the form the original opens both modes with, so each mode keeps
 *           that assignment outside its loop.
 *           Lever evidence: the range is code, so the data promotion of the
 *           0x801F442C pointer run it indexes was declined, and no fragment
 *           boundary reaches it because the body carries its own 0x20-byte
 *           frame and its own return. No byte-identical twin exists, so clone
 *           stayed ineligible.
 * @status partial
 * @match 82.03
 * @residual Live comparison is 105 of 128 instructions at 512 bytes against
 *           the original's 127 at 508, so this body emits one instruction the
 *           original does not: in mode 0 the table-derived point reaches the
 *           probe's second argument through a move out of $v0, where the
 *           original builds that value in $a1 in the first place. Everything
 *           else is register naming and ordering inside the answered-record
 *           block of both modes: the original issues the scratchpad cell load
 *           before the offset shift and reads the record word into $v0 ahead of
 *           the shared camera word, while this body shifts the offset first
 *           and swaps those two registers, which also swaps the operands of
 *           the final subtraction. The probe call, the three gates, the stride
 *           build, the octant store pair and the action request agree
 *           instruction for instruction.
 */
void probeAxisPointRunAndSetFoundRecordOctantSideThenRequestAction2World03Area13913_801F308C(
    u8 axis) {
  u8* work;
  u8* run;
  u8 i;
  s32 target;
  s32 point;
  s32 value;
  s32 off;

  if (axis == 0) {
    i = 0;
    run = D_801F4440;
    for (; i < 4; i++) {
      work = D_1F800044;
      FIELD_REF(s32, work, 0x34) = 0x5C0000;
      point = run[i] << 16;
      FIELD_REF(s32, work, 0x38) = point;
      target = func_801BE0C0(FIELD_REF(s32, work, 0x34), point, 0);
      if ((u8)target == 0xFF) {
        continue;
      }
      if ((D_8014625A & 0x1400) != 0) {
        continue;
      }
      if (D_80143BB0 != 0) {
        continue;
      }
      off = (((u8)target << 2) + (u8)target) << 6;
      value = FIELD_REF(s32, D_1F800044, 0x34) -
          FIELD_REF(s32, D_80145EC4, off);
      if (value > 0) {
        FIELD_REF(u8, D_80145E98, off) = 3;
      } else {
        FIELD_REF(u8, D_80145E98, off) = 7;
      }
      func_8015C214(target, 2);
    }
  } else if (axis == 1) {
    i = 0;
    run = D_801F4444;
    for (; i < 4; i++) {
      work = D_1F800044;
      point = run[i] << 16;
      FIELD_REF(s32, work, 0x34) = point;
      FIELD_REF(s32, work, 0x38) = 0x440000;
      target = func_801BE0C0(FIELD_REF(s32, work, 0x34), 0x440000, 0);
      if ((u8)target == 0xFF) {
        continue;
      }
      if ((D_8014625A & 0x1400) != 0) {
        continue;
      }
      if (D_80143BB0 != 0) {
        continue;
      }
      off = (((u8)target << 2) + (u8)target) << 6;
      value = FIELD_REF(s32, D_1F800044, 0x38) -
          FIELD_REF(s32, D_80145EC8, off);
      if (value > 0) {
        FIELD_REF(u8, D_80145E98, off) = 5;
      } else {
        FIELD_REF(u8, D_80145E98, off) = 1;
      }
      func_8015C214(target, 2);
    }
  }
}
