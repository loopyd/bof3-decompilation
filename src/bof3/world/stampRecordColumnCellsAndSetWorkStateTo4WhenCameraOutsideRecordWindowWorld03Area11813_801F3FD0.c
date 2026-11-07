#include "bof3/bof3.h"

/* The overlay record draw routine this step finishes with, called with a zero
 * mode argument. It is a later code span of this same overlay. */
void func_801F381C(s32 arg0);

/* Shared camera coordinate words of the main executable, projected by the world
 * overlays. Both lie outside this overlay's payload and are D_<ADDRESS> names,
 * so the generated undefined-syms map binds them and no target-map row is
 * needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Resident map base pointer and map row stride byte of the main executable,
 * the same pair func_801F4610 stamps through. Both outside this overlay's
 * payload and bound by their D_<ADDRESS> names. */
extern u32 D_8014931C; /* @source 0x8014931C @kind unknown */
extern u8 D_80104000;  /* @source 0x80104000 @kind unknown */

/* Shared overlay state byte of the main executable. This step only reads it. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* @source 0x801F3FD0
 * @behavior Byte-2 state handler that the overlay's second byte-2 dispatch
 *           table (0x801F5184, indexed by the dispatcher at 0x801F3C10) lists
 *           at entry 3. Reads the work record published at the scratchpad
 *           pointer cell 0x1F800044 and compares its byte at offset 0x0A with
 *           the shared overlay state byte 0x8014933B: when they are equal the
 *           body is skipped and only the draw call runs. Otherwise it tests the
 *           camera against the record's cell window. The shared camera word
 *           0x80145EC4 is compared with the record halfword at offset 0x36
 *           through that cell's centre ((halfword + 1) << 16) | 0x8000, and the
 *           shared camera word 0x80145EC8 is compared with the record halfword
 *           at offset 0x3A through both the cell start (halfword << 16) and the
 *           next cell start ((halfword + 1) << 16). An absolute difference
 *           above 0x20000 on the first comparison, or above it on both of the
 *           other two, puts the camera outside the window. Outside, it stamps
 *           0x52 into the map byte of the record's column at offset 0x36 for
 *           the row at offset 0x3A and again for the row below, addressed
 *           through the resident map base pointer 0x8014931C scaled by the
 *           resident map row stride byte 0x80104000, and then stores 4 into the
 *           record state byte at offset 0x02 so the next dispatch takes entry 4
 *           of the same table. In every path it finishes with the overlay record
 *           draw routine 0x801F381C called with a zero mode argument. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps $ra
 *           at 0x10 across that call.
 * @status partial
 * @match 74.68
 * @residual The body is 316 bytes like the original. The first camera load
 *           and cell-centre arithmetic use different registers, but the first
 *           difference result now uses the original register. The marker,
 *           second record reload and state-byte reload still differ in
 *           allocation. The stride address is rematerialized for each load
 *           rather than retained as a full address across both stamps.
 */
void stampRecordColumnCellsAndSetWorkStateTo4WhenCameraOutsideRecordWindowWorld03Area11813_801F3FD0(
    void) {
  u8* work;
  s32 limit;
  s32 col;
  u8* record;
  s32 base;

  work = SPAD_ADDR(u8, 0x44u);
  work = *(u8**)work;
  limit = 0x20000;
  if (D_8014933B == FIELD_REF(u8, work, 0x0A)) {
    goto draw;
  }

  col = FIELD_REF(s16, work, 0x36);
  {
    s32 d;

    base = ((col + 1) << 16) | 0x8000;
    d = D_80145EC4 - base;
    if (d < 0) {
      d = -d;
    }
    d = d > limit;
    if (d) {
      goto stamp;
    }
  }

  {
    s32 d;

    base = D_80145EC8;
    d = base - (FIELD_REF(s16, work, 0x3A) << 16);
    if (d < 0) {
      d = -d;
    }
    d = d > limit;
    if (d) {
      d = base - ((FIELD_REF(s16, work, 0x3A) + 1) << 16);
      if (d < 0) {
        d = -d;
      }
      d = d > limit;
      if (d) {
stamp:
        *(u8*)(D_8014931C + FIELD_REF(s16, work, 0x3A) * D_80104000 + col) = 0x52;
        record = SPAD_ADDR(u8, 0x44u);
        record = *(u8**)record;
        *(u8*)(D_8014931C + (FIELD_REF(s16, record, 0x3A) + 1) * D_80104000 +
               FIELD_REF(s16, record, 0x36)) = 0x52;
        work = SPAD_ADDR(u8, 0x44u);
        work = *(u8**)work;
        FIELD_REF(u8, work, 0x02) = 4;
      }
    }
  }

draw:
  func_801F381C(0);
}
