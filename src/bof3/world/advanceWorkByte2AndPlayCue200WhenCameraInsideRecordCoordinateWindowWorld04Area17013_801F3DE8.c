#include "bof3/bof3.h"

/* Shared camera coordinate words of the main executable, published by the
 * front-end selectors and projected by the world overlays. Both lie outside
 * this overlay's payload; Splat binds them through this target's generated
 * undefined-syms map, so no target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Front-end phase byte of the main executable, outside this overlay's
 * payload. The cue below only fires while the front end holds phase 0. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Sound-cue dispatcher of the main executable, outside this overlay's
 * payload. */
void func_8015DF18(u16 cue);

/* Record draw routine of this overlay, still an `asm` span of this target at
 * 0x801F40F0; it re-reads the same work record and builds the sprite
 * primitive. */
void drawRouteAxisSplitTexturedQuadPairAtWorkCellScaledByHalfword30World04Area17013_801F40F0(
    void);

/* @source 0x801F3DE8
 * @behavior Byte-2 state handler, entry 1 of the five-entry handler table based
 *           at 0x801F588C, the table the overlay dispatcher at 0x801F3BE8
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044. Reads the
 *           record's route byte at +8 once: when it is non-zero the gate
 *           compares the shared camera word D_80145EC8 with the record's
 *           halfword at +0x3A, and when it is zero it compares D_80145EC4 with
 *           the record's halfword at +0x36 instead. Both comparisons build the
 *           cell centre (((halfword + 1) << 16) | 0x8000) and take the
 *           absolute difference from the camera word; when that difference
 *           exceeds 0x8000 the record is off screen and only the draw routine
 *           runs. Otherwise the record halfword of the other axis (0x36 on the
 *           non-zero route, 0x3A on the zero route) and the matching camera
 *           word become the shared tail operands: the record is inside the
 *           camera window when the absolute difference from either the
 *           halfword's own cell start (halfword << 16) or the next cell start
 *           ((halfword + 1) << 16) stays within 0x10000. Inside that window
 *           the front-end phase byte D_80143BB0 gates the sound cue 0x200
 *           through func_8015DF18, and the record's step byte at +2, the byte
 *           the overlay dispatcher indexes, is advanced by one so the next
 *           dispatch takes entry 2 at 0x801F3F28. The draw routine at
 *           0x801F40F0 runs on every call, including the frames that stay off
 *           screen, and re-reads the same record. The pointer cell is re-read
 *           after the cue gate, both times as `lui` plus the offset load.
 *           Takes no arguments and returns nothing; the 0x18-byte frame only
 *           keeps $ra at 0x10.
 *
 * Name evidence: emi/world04/area162/13 lifts the same body exactly at
 * 0x801F2DB8 as advanceWorkByte2AndPlayCue200WhenCameraInsideRecordCoordinate
 * Window, and area163/13 and area164/13 carry that spelling at their own
 * 0x801F2DB8. A payload compare of the 0x140-byte spans found 2 differing
 * words of 80, both relocation words: the in-body `j` whose target shifts by
 * exactly the 0x1030 address delta between the two spans, and the `jal` to
 * each overlay's own draw routine (0x801F30D4 there, 0x801F40F0 here). That
 * fixes the window limits, the inside-window polarity, the cue value and the
 * byte-2 advance; only this overlay's draw callee and address differ, so the
 * name transfers with the target and address suffix.
 *
 * Shape evidence: entry 3 at 0x801F3F84 and entry 4 at 0x801F4080 of the same
 * handler table are exact lifts of this target and fix the record accessor
 * chain, the shared tail operand pair, the D_80143BB0 cue gate and the empty
 * tail-call delay slot. Neither is clone-eligible for this span: 0x801F3F84
 * gates with one 0x20000 limit and advances outside the window, and
 * 0x801F4080 replaces the gate with a halfword countdown and the advance with
 * a stored state of 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2AndPlayCue200WhenCameraInsideRecordCoordinateWindowWorld04Area17013_801F3DE8(
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
      goto draw;
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
      goto draw;
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
        goto draw;
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

draw:
  drawRouteAxisSplitTexturedQuadPairAtWorkCellScaledByHalfword30World04Area17013_801F40F0();
}
