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

/* Record draw routine of this overlay, still an `asm` span of this target. */
void func_801F3F84(void);

/* @source 0x801F3C68
 * @behavior Byte-2 state handler entry 1 of the five-entry handler table based
 *           at 0x801F4448, the table the overlay dispatcher at 0x801F3A8C
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044. Reads that
 *           record's route byte at offset 0x08 once: when it is non-zero the
 *           gate compares the shared camera word 0x80145EC8 with the record
 *           halfword at offset 0x3A, and when it is zero it compares
 *           0x80145EC4 with the halfword at offset 0x36 instead. Both build
 *           the cell centre (((halfword + 1) << 16) | 0x8000) and take the
 *           absolute difference from the camera word; a difference above
 *           0x8000 leaves only the draw routine. Otherwise the record halfword
 *           of the other axis (0x36 on the non-zero route, 0x3A on the zero
 *           route) and its camera word (0x80145EC4 and 0x80145EC8) measure the
 *           same distance from the halfword's own cell start and from the next
 *           cell's start against 0x10000. Inside that window the front-end
 *           phase byte 0x80143BB0 gates the sound cue 0x200 through the
 *           main-executable dispatcher 0x8015DF18, and the record state byte at
 *           offset 0x02, the byte the overlay dispatcher indexes, is advanced
 *           by one past a fresh read of the scratchpad cell, so the next
 *           dispatch takes entry 2 at 0x801F3DA8. Every path ends by calling
 *           the payload record draw routine at 0x801F3F84, which imports
 *           SetDrawMode, SetPolyFT4, SetShadeTex and RotTransPers4. Takes no
 *           arguments and returns nothing; the 0x18-byte frame keeps $ra at
 *           0x10 and the first pointer load precedes the frame creation.
 *
 *           Shape evidence: a little-endian word compare of this 0x140-byte
 *           span against the payloads under out/binaries found the same body at
 *           0x801F2DB8 of World04/Area16213, Area16313 and Area16413, each
 *           differing in exactly two words, the internal jump target at +0x64
 *           and the draw `jal` at +0x128, and both differ from this span's
 *           words by exactly the 0xEB0 address delta between the spans, so the
 *           spans carry one body at two addresses; the spans are not
 *           byte-identical, so clone was declined and the World04/Area16213
 *           source, whose own 0x140-byte span measured exact, supplied the
 *           statement order, the pointer-local spelling and the reload points.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte2AndPlayCue200WhenCameraInsideRecordCoordinateWindowWorld03Area13913_801F3C68(
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
  func_801F3F84();
}
