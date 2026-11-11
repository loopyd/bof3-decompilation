#include "bof3/bof3.h"

/* Shared camera coordinate words of the main executable, published by the
 * front-end selectors and projected by the world overlays. Both lie outside
 * this overlay's payload; Splat binds them through this target's generated
 * undefined-syms map, so no target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Local routine of this overlay at 0x801F3108; its role is not established. */
void func_801F3108(void);

/* @source 0x801F2F88
 * @behavior Byte-2 state handler 3 of the local handler table based at
 *           0x801F38CC, the word that table stores at 0x801F38D8, for the
 *           work record published at the scratchpad pointer cell 0x1F800044,
 *           followed by the overlay's own routine at 0x801F3108. Reads the
 *           record's route byte at +8 once: when it is non-zero the gate
 *           compares the shared camera word D_80145EC8 with the record's
 *           halfword at +0x3A, and when it is zero it compares
 *           D_80145EC4 with the record's halfword at +0x36 instead. Both
 *           comparisons build the cell centre (((halfword + 1) << 16) |
 *           0x8000), take the absolute difference from the camera word and
 *           advance the record's step byte at +2 when that difference
 *           exceeds 0x20000. When the first check passes, the record
 *           halfword of the other axis (0x36 on the non-zero route, 0x3A on
 *           the zero route) and the matching camera word become the shared
 *           tail's operands: the step byte is advanced when the absolute
 *           difference from both the halfword's own cell start (halfword
 *           << 16) and the next cell's start ((halfword + 1) << 16) exceeds
 *           0x20000. The routine at 0x801F3108 runs either way. Takes no
 *           arguments and returns nothing; the frame is 0x18 bytes and only
 *           keeps $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2WhenCameraOutsideRecordCoordinateWindowWorld04Area15713_801F2F88(
    void) {
  u8* work;
  s32 value;
  s32 base;
  s32 limit;

  /* Reuse the consumed cell address for its loaded record pointer. Keeping
   * this real address-to-record chain preserves the original register web. */
  work = SPAD_ADDR(u8, 0x44u);
  work = *(u8**)work;
  limit = 0x20000;

  if (work[8] != 0) {
    s32 d;

    d = D_80145EC8 - (((FIELD_REF(s16, work, 0x3A) + 1) << 16) | 0x8000);
    if (d < 0) {
      d = -d;
    }
    d = d > limit;
    if (d) {
      goto advance;
    }
    value = FIELD_REF(s16, work, 0x36);
    base = D_80145EC4;
  } else {
    s32 d;

    d = D_80145EC4 - (((FIELD_REF(s16, work, 0x36) + 1) << 16) | 0x8000);
    if (d < 0) {
      d = -d;
    }
    d = d > limit;
    if (d) {
      goto advance;
    }
    value = FIELD_REF(s16, work, 0x3A);
    base = D_80145EC8;
  }

  {
    s32 d;

    d = base - (value << 16);
    if (d < 0) {
      d = -d;
    }
    d = d > limit;
    if (d) {
      d = base - ((value + 1) << 16);
      if (d < 0) {
        d = -d;
      }
      d = d > limit;
      if (d) {
advance:
        work[2]++;
      }
    }
  }

  func_801F3108();
}
