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

/* Local drawing routine of this overlay (still an `asm` span of this target). */
void func_801F30D4(void);

/* @source 0x801F2DB8
 * @behavior Overlay step gate over the scratchpad work record at
 *           0x1F800044, followed by the overlay's draw routine. Reads the
 *           record's route byte at +8 once: when it is non-zero the gate
 *           compares the shared camera word D_80145EC8 with the record's
 *           halfword at +0x3A, and when it is zero it compares
 *           D_80145EC4 with the record's halfword at +0x36 instead. Both
 *           comparisons build the cell centre (((halfword + 1) << 16) |
 *           0x8000) and take the absolute difference from the camera word;
 *           when that difference exceeds 0x8000 the record is off screen and
 *           only the draw routine runs. Otherwise the record halfword of the
 *           other axis (0x36 on the non-zero route, 0x3A on the zero route)
 *           and the matching camera word become the shared tail's operands:
 *           the record is inside the camera window when the absolute
 *           difference from either the halfword's own cell start
 *           (halfword << 16) or the next cell's start ((halfword + 1) << 16)
 *           stays within 0x10000. Inside that window the front-end phase byte
 *           D_80143BB0 gates the sound cue 0x200 through func_8015DF18, and
 *           the record's step byte at +2, the byte the overlay dispatcher
 *           indexes, is advanced by one. Takes no arguments and returns
 *           nothing; the frame is 0x18 bytes and only keeps $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2AndPlayCue200WhenCameraInsideRecordCoordinateWindowWorld04Area16413_801F2DB8(
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
    /* The conditional-expression magnitude keeps the negation inside this
     * arm, so the 0x8000 compare and its constant stay in the shared tail and
     * the arm's branch keeps its `nop`; the `if (d < 0) d = -d;` form sinks
     * that constant into the branch delay slot. */
    d = d >= 0 ? d : -d;
    if (d > 0x8000) {
      goto draw;
    }
    value = FIELD_REF(s16, work, 0x36);
    base = D_80145EC4;
    /* The cell span is materialised once per arm, immediately before the
     * shared tail. */
    limit = 0x10000;
  } else {
    s32 d;

    d = D_80145EC4 - (((FIELD_REF(s16, work, 0x36) + 1) << 16) | 0x8000);
    d = d >= 0 ? d : -d;
    if (d > 0x8000) {
      goto draw;
    }
    value = FIELD_REF(s16, work, 0x3A);
    base = D_80145EC8;
    limit = 0x10000;
  }

  {
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
  /* The cue call may re-publish the work record, so the pointer cell is read
   * again and the new record keeps its own quantity. */
  {
    u8* record;

    record = *(u8**)SPAD_ADDR(u8, 0x44u);
    record[2]++;
  }

draw:
  func_801F30D4();
}
