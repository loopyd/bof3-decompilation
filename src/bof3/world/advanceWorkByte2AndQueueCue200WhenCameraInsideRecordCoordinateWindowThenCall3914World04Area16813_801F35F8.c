#include "bof3/bof3.h"

/* Shared camera coordinate words of the main executable, published by the
 * front-end selectors and projected by the world overlays. Both lie outside
 * this overlay's payload; Splat binds them through this target's generated
 * undefined-syms map, so no target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Front-end phase byte of the main executable, outside this overlay's payload
 * and already mapped by the shared symbol map. The cue below only fires while
 * the front end holds phase 0. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Sound-cue dispatcher of the main executable; the exact sibling handlers pass
 * it a cue id and discard any result.
 * @source 0x8015DF18 @kind function
 */
extern void func_8015DF18(u16 cue);

/* Update routine of this payload, still an `asm` span at 0x801F3914: it re-reads
 * the same work record, writes scratchpad and GTE state, and answers nothing.
 * @source 0x801F3914 @kind function
 */
extern void func_801F3914(void);

/* @source 0x801F35F8
 * @behavior Byte-2 state handler of this overlay, reached through the handler run
 *           based at 0x801F4B2C: the body address appears once in the payload
 *           bytes, at word 0x801F4B8C, entry 24 of that run, so no jal in this
 *           payload names it, it takes no arguments and it answers no value.
 *           Reads the scratchpad work
 *           record at 0x1F800044 once through its pointer cell: the record's
 *           route byte at +8 selects the axis pair, where non-zero compares the
 *           shared camera word D_80145EC8 with record halfword +0x3A and zero
 *           compares D_80145EC4 with +0x36. Each arm builds the cell centre
 *           (((halfword + 1) << 16) | 0x8000) and takes the absolute distance
 *           from its camera word; past 0x8000 the record is off screen and only
 *           the local helper 0x801F3914 runs. Otherwise the other axis
 *           (+0x36 on the non-zero route, +0x3A on the zero route) and its
 *           camera word become the shared tail's operands, which accepts the
 *           record as inside the window when either the absolute distance from
 *           the cell start (halfword << 16) or from the next cell start
 *           ((halfword + 1) << 16) stays within 0x10000. Inside that window the
 *           front-end phase byte D_80143BB0 gates sound cue 0x200 through
 *           func_8015DF18, that cue firing only while the byte reads zero, and
 *           the record's state byte at +2 - the byte the overlay dispatcher
 *           indexes - is advanced by one through a second read of the same
 *           pointer cell, because the cue call clobbers the first pointer. The
 *           local helper 0x801F3914 runs after that store on every path. Takes
 *           no arguments and returns nothing; the 0x18-byte frame only keeps
 *           $ra at 0x10 and the delay slots after the route test, after each
 *           distance branch and before the tail call stay nops. The roles of the
 *           0x36 and 0x3A halfwords and of the several distance windows stay
 *           address-anchored.
 *
 * Name evidence: a payload cmp of this 0x140-byte span against the published
 * exact lift at emi/world04/area162/13@0x801F2DB8 differs in exactly two words,
 * and both differ by that span's own 0x840 address delta: the internal jump at
 * +0x64 names 0x801F36AC here and 0x801F2E6C there, and the tail call at +0x128
 * names 0x801F3914 here and 0x801F30D4 there, each payload's own helper. Equal
 * deltas across every differing word prove one body, so the sibling supplies the
 * statement order, operand forms and the inside-window topology, while the tail
 * helper is re-derived from this payload's own bytes. The sibling's wording for
 * the callee is its draw routine and does not transfer: 0x801F3914 is this
 * overlay's update routine, as the two exact neighbours at 0x801F3738 and
 * 0x801F3794 record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2AndQueueCue200WhenCameraInsideRecordCoordinateWindowThenCall3914World04Area16813_801F35F8(
    void) {
  u8* work;
  s32 value;
  s32 base;
  s32 limit;

  /* Reuse the consumed cell address for its loaded record pointer. Keeping
   * this real address-to-record chain preserves the original register web. */
  work = SPAD_ADDR(u8, 0x44u);
  work = *(u8**)work;

  if (work[8] != 0) {
    s32 d;

    d = D_80145EC8 - (((FIELD_REF(s16, work, 0x3A) + 1) << 16) | 0x8000);
    /* The integer absolute-value intrinsic keeps the sign branch and its
     * empty delay slot inside one compiler operation. */
    d = __builtin_abs(d);
    if (d > 0x8000) {
      goto update;
    }
    /* Each arm materialises the shared tail's 0x10000 window: the original
     * stores that `lui` in the arm's own branch delay slot, so it is defined
     * here rather than once before the arms. */
    limit = 0x10000;
    value = FIELD_REF(s16, work, 0x36);
    base = D_80145EC4;
  } else {
    s32 d;

    d = D_80145EC4 - (((FIELD_REF(s16, work, 0x36) + 1) << 16) | 0x8000);
    /* The integer absolute-value intrinsic keeps the sign branch and its
     * empty delay slot inside one compiler operation. */
    d = __builtin_abs(d);
    if (d > 0x8000) {
      goto update;
    }
    limit = 0x10000;
    value = FIELD_REF(s16, work, 0x3A);
    base = D_80145EC8;
  }

  {
    /* The tail keeps its halfword cell-start term inline in each subtraction,
     * because the original recomputes it per guard. */
    s32 d;

    d = base - (value << 16);
    if (d < 0) {
      d = -d;
    }
    if (d > limit) {
      d = base - ((value + 1) << 16);
      if (d < 0) {
        d = -d;
      }
      if (d > limit) {
        goto update;
      }
    }
  }

  if (D_80143BB0 == 0) {
    func_8015DF18(0x200);
  }
  {
    /* The original reloads the cursor cell after the cue call and keeps that
     * second pointer in its own register, so it takes a variable of its own
     * instead of reusing the arm's record pointer. */
    u8* cursor;

    cursor = *(u8**)SPAD_ADDR(u8, 0x44u);
    cursor[2]++;
  }

update:
  func_801F3914();
}
