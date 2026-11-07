#include "bof3/bof3.h"

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

/* @source 0x801F3590
 * @behavior Overlay step gate over the scratchpad work record at 0x1F800044.
 *           When the shared front-side byte D_80143F03 reads zero, the record
 *           pointer is loaded once to compare the record's byte at +0xA with
 *           the shared overlay state byte D_8014933B; when they are equal the
 *           step returns without touching the record. Otherwise the record
 *           pointer is loaded again and its route byte at +8 read once: when
 *           it is non-zero the gate compares the shared camera word
 *           D_80145EC8 with the record's halfword at +0x3A, and when it is
 *           zero it compares D_80145EC4 with the record's halfword at +0x36
 *           instead. Both comparisons build the cell centre
 *           (((halfword + 1) << 16) | 0x8000), take the absolute difference
 *           from the camera word and store 4 into the record's state byte at
 *           +2 when that difference exceeds 0x20000. When the first check
 *           passes, the record halfword of the other axis (0x36 on the
 *           non-zero route, 0x3A on the zero route) and the matching camera
 *           word become the shared tail's operands: the state byte is set to 4
 *           when the absolute difference from both the halfword's own cell
 *           start (halfword << 16) and the next cell's start
 *           ((halfword + 1) << 16) exceeds 0x20000. Takes no arguments, calls
 *           nothing, returns nothing and keeps no frame; the state byte it
 *           writes is the byte the overlay dispatcher indexes.
 * @status partial
 * @match 95.89
 * @residual The body and the shared tail match instruction for instruction.
 *           The three instructions of the leading gate differ only in the
 *           register that holds the loaded record pointer: the original keeps
 *           it in $v0 for the +0xA comparison and reloads into $a0 for the
 *           body, while one shared pointer variable makes both uses $a0.
 *           Splitting the gate pointer into its own variable reproduces $v0
 *           but swaps the body's $a0/$a1 pair ($a1 for the record, $a0 for the
 *           camera word), so both source shapes miss one allocation choice.
 */
void setWorkState2To4WhenCameraOutsideRecordCoordinateWindowWorld03Area11813_801F3590(
    void) {
  u8* work;
  s32 value;
  s32 base;
  s32 limit;

  if (D_80143F03 == 0) {
    work = SPAD_ADDR(u8, 0x44u);
    work = *(u8**)work;
    if (D_8014933B == FIELD_REF(u8, work, 0xA)) {
      return;
    }
  }

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
        work[2] = 4;
      }
    }
  }
}
