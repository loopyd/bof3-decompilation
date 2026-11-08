#include "bof3/bof3.h"

/* Work record pointer cell of the scratchpad. The original re-materialises the
 * cell address and reloads the pointer once per access group, so the
 * fixed-address cell keeps its own loads. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Shared camera coordinate words of the main executable, published by the
 * front-end selectors and projected by the world overlays. Both lie outside
 * this overlay's payload; Splat binds them through this target's generated
 * undefined-syms map, so no target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Shared front-side byte and shared overlay state byte of the main executable,
 * both outside this overlay's payload and covered by the same generated map.
 * This step only reads them, so they are bound with plain byte externs. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Front-end phase byte of the main executable; the cue below only fires while
 * that phase holds zero. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

/* The overlay record draw routine this step always finishes with, called with
 * mode 1. It is a later code span of this same overlay. */
void func_801F381C(s32 arg0);

/* @source 0x801F33A4
 * @behavior Byte-2 state handler that the first byte-2 dispatch table of this
 *           overlay (0x801F516C, indexed by the dispatcher at 0x801F3014) lists
 *           at entry 1; that dispatcher indexes the work record published at
 *           the scratchpad pointer cell 0x1F800044 by its state byte at offset
 *           0x02. When the shared front-side byte 0x80143F03 reads zero and the
 *           record byte at offset 0x0A equals the shared state byte 0x8014933B,
 *           the step queues sound cue 0x200 through the resident cue dispatcher
 *           0x8015DF18 while the front-end phase byte 0x80143BB0 is zero and
 *           stores 2 into that state byte. The record pointer is loaded again
 *           for the body, which reads the record route byte at offset 0x08
 *           once: a non-zero route compares the camera word 0x80145EC8 with
 *           the cell centre of the record halfword at 0x3A, and a zero route
 *           compares 0x80145EC4 with the centre of the halfword at 0x36, the
 *           centre being (((halfword + 1) << 16) | 0x8000). When that absolute
 *           difference exceeds 0x8000 the record is off screen and only the
 *           draw routine runs. Otherwise the halfword of the other axis (0x36
 *           on the non-zero route, 0x3A on the zero route) and the matching
 *           camera word become the shared tail's operands: the state byte is
 *           set to 2 when the absolute difference from either that halfword
 *           cell start (halfword << 16) or the next cell start
 *           ((halfword + 1) << 16) stays within 0x10000, which keeps the
 *           record in the approach state and queues cue 0x200 on the same
 *           phase gate. Every path finishes with the overlay record draw
 *           routine 0x801F381C called with mode 1. Takes no arguments, returns
 *           nothing, keeps a 0x18-byte frame only for the return address at
 *           0x10, and reloads the record cell for each of its four uses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkState2To2AndQueueCue200WhenCameraInsideRecordCoordinateWindowThenDrawMode1World03Area11813_801F33A4(
    void) {
  u8* work;
  s32 value;
  s32 base;
  s32 limit;

  if (D_80143F03 == 0 &&
      D_8014933B == FIELD_REF(u8, D_1F800044, 0x0A)) {
    if (D_80143BB0 == 0) {
      dispatchSoundCue(0x200);
    }
    FIELD_REF(u8, D_1F800044, 0x02) = 2;
  }

  work = D_1F800044;

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
     * stores that load in the arm's own branch delay slot, so it is defined
     * here rather than once before the arms. */
    limit = 0x10000;
    value = FIELD_REF(s16, work, 0x36);
    base = D_80145EC4;
  } else {
    s32 d;

    d = D_80145EC4 - (((FIELD_REF(s16, work, 0x36) + 1) << 16) | 0x8000);
    d = __builtin_abs(d);
    if (d > 0x8000) {
      goto draw;
    }
    limit = 0x10000;
    value = FIELD_REF(s16, work, 0x3A);
    base = D_80145EC8;
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
    dispatchSoundCue(0x200);
  }
  FIELD_REF(u8, D_1F800044, 0x02) = 2;
draw:
  func_801F381C(1);
}
