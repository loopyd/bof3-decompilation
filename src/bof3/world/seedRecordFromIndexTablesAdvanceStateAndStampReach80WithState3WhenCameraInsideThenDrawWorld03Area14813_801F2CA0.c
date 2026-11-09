#include "bof3/bof3.h"

/* Shared work-record pointer cell of the scratchpad. The original
 * re-materialises the cell address and reloads the pointer once per access
 * group, so the fixed-address cell keeps its own loads. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Per-index seed tables of this overlay's own data span at 0x801F4FD8. Each is
 * an interior word of that reviewed run and a D_<ADDRESS> name, so the
 * generated undefined-syms map binds it and no target-map row is needed. The
 * bytes at 0x801F5004 and 0x801F5005 are the two halves of one two-byte seed
 * entry, and 0x801F5010 is the byte run the record halfword at 0x32 receives. */
extern u16 D_801F4FD8[]; /* @source 0x801F4FD8 @kind data */
extern u16 D_801F4FE4[]; /* @source 0x801F4FE4 @kind data */
extern u32 D_801F4FF0[]; /* @source 0x801F4FF0 @kind data */
extern u8 D_801F5004[];  /* @source 0x801F5004 @kind data */
extern u8 D_801F5005[];  /* @source 0x801F5005 @kind data */
extern u8 D_801F5010[];  /* @source 0x801F5010 @kind data */

/* Shared camera coordinate words of the main executable, projected by the
 * world overlays. Both lie outside this overlay's payload and are
 * D_<ADDRESS> names, so the generated undefined-syms map binds them and no
 * target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* The overlay draw routine this step finishes with, called with a zero mode
 * argument. It is a later code span of the same overlay and carries its own
 * target-map row. */
void func_801F31AC(s8 arg0);

/* @source 0x801F2CA0
 * @behavior Byte-2 state handler that the overlay's first state-byte-2
 *           dispatch table (0x801F4FC4, indexed by the dispatcher at
 *           0x801F2C5C) lists at entry 0. Reads the work record published at
 *           the scratchpad pointer cell 0x1F800044 and seeds it from this
 *           overlay's own per-index tables: the byte at 0x801F5004 indexed by
 *           twice the signed halfword at offset 0x36 goes into the halfword at
 *           0x36, the byte at 0x801F5005 at that same index into the halfword
 *           at 0x3A, the halfword at 0x801F4FD8 at that index into the
 *           halfword at 0x3E, the word at 0x801F4FF0 at that index into the
 *           word at 0x20, the halfword at 0x801F4FE4 at that index into the
 *           halfword at 0x2E and the byte at 0x801F5010 at the plain index
 *           into the halfword at 0x32; it also records whether the original
 *           halfword at offset 0x3A was zero in the byte at offset 0x08,
 *           clears the halfword at 0x30 and increments the record state byte
 *           at offset 0x02 so the next dispatch takes entry 1 of the same
 *           table. It then re-reads the route byte at 0x08: when it is
 *           non-zero the window test takes the shared camera word 0x80145EC8
 *           against the cell centre ((halfword at 0x3A plus 1) << 16) or
 *           0x8000 and keeps 0x80145EC4 with the halfword at 0x36 as the
 *           shared tail operands, and when it is zero it takes 0x80145EC4
 *           against the centre of the halfword at 0x36 and keeps 0x80145EC8
 *           with the halfword at 0x3A instead. When that first distance is no
 *           further than 0x8000 and the shared camera word lies no further
 *           than 0x10000 from the halfword's own cell start (halfword << 16)
 *           or from the next cell start ((halfword + 1) << 16), it stamps 0x80
 *           into the record halfword at 0x30 and 3 into the state byte at
 *           0x02. Every path finishes with the overlay draw routine 0x801F31AC
 *           called with a zero mode argument. Takes no arguments and returns
 *           nothing; the record pointer is loaded before the frame is created
 *           and the 0x18-byte frame keeps $ra at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRecordFromIndexTablesAdvanceStateAndStampReach80WithState3WhenCameraInsideThenDrawWorld03Area14813_801F2CA0(
    void) {
  u8* work;
  u8* gate;
  s32 index;
  s32 mode;
  s32 route;
  s32 value;
  s32 base;
  s32 limit;

  route = FIELD_REF(s16, D_1F800044, 0x3A) == 0;
  index = FIELD_REF(s16, D_1F800044, 0x36);
  FIELD_REF(u8, D_1F800044, 0x08) = route;

  work = D_1F800044;
  FIELD_REF(u16, work, 0x36) = D_801F5004[index * 2];
  FIELD_REF(u16, work, 0x3A) = D_801F5005[index * 2];
  FIELD_REF(u16, work, 0x3E) = D_801F4FD8[index];
  FIELD_REF(u32, work, 0x20) = D_801F4FF0[index];
  FIELD_REF(u16, work, 0x2E) = D_801F4FE4[index];
  mode = D_801F5010[index];
  ++FIELD_REF(u8, work, 0x02);
  gate = D_1F800044;
  FIELD_REF(u16, work, 0x30) = 0;
  FIELD_REF(u16, work, 0x32) = mode;

  if (FIELD_REF(u8, gate, 0x08) != 0) {
    s32 diff;

    diff = D_80145EC8 - (((FIELD_REF(s16, gate, 0x3A) + 1) << 16) | 0x8000);
    /* The integer intrinsic keeps the sign branch and its empty delay slot
     * together; the shared tail uses ordinary branch-based absolute values. */
    diff = __builtin_abs(diff);
    if (diff > 0x8000) {
      goto draw;
    }
    limit = 0x10000;
    value = FIELD_REF(s16, gate, 0x36);
    base = D_80145EC4;
  } else {
    s32 diff;

    diff = D_80145EC4 - (((FIELD_REF(s16, gate, 0x36) + 1) << 16) | 0x8000);
    /* The integer intrinsic keeps the sign branch and its empty delay slot
     * together; the shared tail uses ordinary branch-based absolute values. */
    diff = __builtin_abs(diff);
    if (diff > 0x8000) {
      goto draw;
    }
    limit = 0x10000;
    value = FIELD_REF(s16, gate, 0x3A);
    base = D_80145EC8;
  }

  {
    s32 diff;

    diff = base - (value << 16);
    if (diff < 0) {
      diff = -diff;
    }
    if (diff > limit) {
      diff = base - ((value + 1) << 16);
      if (diff < 0) {
        diff = -diff;
      }
      if (diff > limit) {
        goto draw;
      }
    }
  }

  FIELD_REF(u16, gate, 0x30) = 0x80;
  FIELD_REF(u8, gate, 0x02) = 3;

draw:
  func_801F31AC(0);
}
