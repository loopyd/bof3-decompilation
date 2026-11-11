#include "bof3/bof3.h"

void func_801F3F84(void);

/* Step tables of this overlay's own payload tail. The pair tables at
 * 0x801F4464 and 0x801F4465 hold one byte per step and the two halfword tables
 * at 0x801F445C and 0x801F4460 hold their entries at the doubled index, so a
 * single +0x36 value selects one entry of all four. Nothing outside this
 * payload names them, so they stay address-anchored. */
extern u8 D_801F4464[]; /* @source 0x801F4464 @kind data */
extern u8 D_801F4465[]; /* @source 0x801F4465 @kind data */
extern u16 D_801F445C[]; /* @source 0x801F445C @kind data */
extern u16 D_801F4460[]; /* @source 0x801F4460 @kind data */

/* Shared camera coordinate words of the main executable. Both lie outside this
 * overlay's payload; Splat binds them through this target's generated
 * undefined-syms map, so no target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Scratchpad work-object pointer cell. The original loads this cell three times
 * and keeps the loaded record in a register between them, so the source reads
 * the named cell where the original does and works through the loaded pointer. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F3AD0
 * @behavior Byte-2 state handler entry 0 of the five-entry handler table based
 *           at 0x801F4448, the table the overlay dispatcher at 0x801F3A8C
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044. It seeds the
 *           record from its own step tables: the signed halfword at offset
 *           0x36 selects entry 0x36 * 2 of the byte tables at 0x801F4464 and
 *           0x801F4465 and entry 0x36 of the halfword tables at 0x801F445C and
 *           0x801F4460, whose values overwrite the halfwords at offsets 0x36,
 *           0x3A, 0x3E and 0x2E, and the route byte at offset 0x08 becomes
 *           whether the halfword read at offset 0x3A was zero. It then
 *           increments the state byte at offset 0x02 and zeroes the halfword at
 *           offset 0x30. The gate that follows takes the shared camera word
 *           0x80145EC8 with the halfword at 0x3A when the route byte is set and
 *           0x80145EC4 with the halfword at 0x36 otherwise, builds that axis
 *           value's cell centre (((halfword + 1) << 16) | 0x8000), and returns
 *           through the draw routine when the absolute difference from the
 *           camera word exceeds 0x8000. Otherwise the other axis halfword and
 *           its camera word measure the same distance from the halfword's own
 *           cell start and from the next cell's start against 0x10000; when both
 *           stay within it the halfword at offset 0x30 becomes -0x100 and the
 *           state byte becomes 3, so the next dispatch takes entry 3 at
 *           0x801F3E04. Every path ends by calling the payload record draw
 *           routine at 0x801F3F84. Takes no arguments and returns nothing; the
 *           0x18-byte frame holds $ra at 0x10 and the first pointer load
 *           precedes the frame creation.
 *
 *           Name evidence: the handler table entry order, the state-byte
 *           advance and the draw call are read from this payload's own bytes;
 *           the seam the body crosses is the camera coordinate window, the same
 *           one the World04/Area17013 gate at 0x801F3F84 measures, so the roles
 *           of 0x80145EC4 and 0x80145EC8 stay that gate's. The step tables and
 *           the seeded offsets keep address-anchored names.
 *
 *           Shape evidence: the window gate from 0x801F3B60 to the draw call is
 *           the World04/Area17013 body
 *           advanceWorkByte2WhenCameraOutsideRecordCoordinateWindowWorld04Area17013_801F3F84
 *           with the 0x20000 window narrowed to 0x8000 and 0x10000 and its
 *           verdict inverted, since that gate advances the state byte when it is
 *           outside the window and this one selects state 3 when it is inside;
 *           that source supplied the statement order, the pointer-local
 *           spelling and the reload points. A little-endian masked scan of every
 *           payload under out/binaries with the lui and jump words masked finds
 *           this 0x198-byte body only at 0x801F3AD0 of this payload, so no
 *           byte-identical sibling exists and clone was declined.
 *
 *           Residual: the body compiles to the original 0x198 bytes and 73 of
 *           its 102 instructions match. The first mismatch is instruction 5,
 *           the signed halfword at +0x36: the original keeps that read in the
 *           leading block, in a0, and shifts it in place there, while this
 *           source's allocator sinks the read past the route-byte store, reuses
 *           the head pointer's v1 for it and moves the table pointer to a1, so
 *           the table block's store order follows the allocation instead of the
 *           original's. Splitting the cell reads across a head pointer and a
 *           table pointer (kept here) and moving the read before or after the
 *           route store both measured the same 73 instructions; the two-pointer
 *           variants that also keep the old pointer live across the third read
 *           measured 50 to 62 and 404 bytes. Register placement of that read is
 *           the smallest missing evidence.
 * @status partial
 * @match 71.57
 * @residual Instruction 5: the +0x36 halfword read is sunk past the route-byte
 *           store into v1 instead of staying in the leading block in a0, and the
 *           table block's store order follows that allocation.
 */
void seedHalfwordPairsFromWorkTablesThenAdvanceWorkByte2AndSetThreeInsideCameraWindowThenDrawWorld03Area13913_801F3AD0(
    void) {
  u8* work;
  u8* head;
  s32 index;
  s32 value;
  s32 base;
  s32 limit;

  head = D_1F800044;
  head[8] = FIELD_REF(s16, head, 0x3A) == 0;
  index = FIELD_REF(s16, head, 0x36);

  work = D_1F800044;
  FIELD_REF(u16, work, 0x36) = D_801F4464[index * 2];
  FIELD_REF(u16, work, 0x3A) = D_801F4465[index * 2];
  FIELD_REF(u16, work, 0x3E) = D_801F445C[index];
  work[2]++;
  FIELD_REF(u16, work, 0x30) = 0;
  FIELD_REF(u16, work, 0x2E) = D_801F4460[index];

  work = D_1F800044;
  limit = 0x10000;
  if (work[8] != 0) {
    s32 d;

    d = D_80145EC8 - (((FIELD_REF(s16, work, 0x3A) + 1) << 16) | 0x8000);
    if (d < 0) {
      d = -d;
    }
    if (d > 0x8000) {
      goto done;
    }
    value = FIELD_REF(s16, work, 0x36);
    base = D_80145EC4;
  } else {
    s32 d;

    d = D_80145EC4 - (((FIELD_REF(s16, work, 0x36) + 1) << 16) | 0x8000);
    if (d < 0) {
      d = -d;
    }
    if (d > 0x8000) {
      goto done;
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
      if (d > limit) {
        goto done;
      }
    }
  }

  FIELD_REF(s16, work, 0x30) = -0x100;
  work[2] = 3;
done:
  func_801F3F84();
}
