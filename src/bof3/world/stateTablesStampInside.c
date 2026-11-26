#include "bof3/bof3.h"

/* Shared work-record pointer cell of the scratchpad. The original
 * re-materialises the cell address and reloads the pointer once per access
 * group, so the fixed-address cell keeps its own loads. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Per-index seed tables of this overlay's own data run, the reviewed textbin
 * span based at 0x801F4FAC. Each base is an interior word of that run and a
 * D_<ADDRESS> name, so the generated undefined-syms map binds it and no
 * target-map row is needed. The bytes at 0x801F4FE0 and 0x801F4FE1 are the two
 * halves of one two-byte seed entry; 0x801F4FD0 and 0x801F4FD4 are the
 * halfword runs read into record halfwords 0x3E and 0x2E, and 0x801F4FD8 is
 * the word run read into work word 0x20. */
extern u8 D_801F4FE0[];  /* @source 0x801F4FE0 @kind data */
extern u8 D_801F4FE1[];  /* @source 0x801F4FE1 @kind data */
extern u16 D_801F4FD0[]; /* @source 0x801F4FD0 @kind data */
extern u16 D_801F4FD4[]; /* @source 0x801F4FD4 @kind data */
extern u32 D_801F4FD8[]; /* @source 0x801F4FD8 @kind data */

/* Shared camera coordinate words of the main executable, projected by the
 * world overlays. Both lie outside this overlay's payload and are
 * D_<ADDRESS> names, so the generated undefined-syms map binds them and no
 * target-map row is needed. */
extern s32 D_80145EC4; /* @source 0x80145EC4 @kind data */
extern s32 D_80145EC8; /* @source 0x80145EC8 @kind data */

/* Renderer of the work record, called on every path of this step; its name is
 * this target's own map row for the span at 0x801F3958. */
extern void renderWorkRecord(void);

/* Measured representation notes for the seeding block. Reading the pointer cell
 * as a local pointer slot instead of the cell symbol left the body at 78/107 and
 * stating the state increment before the 0x20 word store left 92/107, because
 * both spellings make the compiler keep the cell address in a register and move
 * the increment above that store. The 0x2E table load has to sit between the
 * 0x20 store and the increment while its store stays after the cleared
 * halfword 0x30: that order reached 107/107, and moving the store before the
 * clear also measured 107/107. */

/* @source 0x801F3490
 * @behavior Byte-2 state handler at entry 0 of this overlay's second handler
 *           run, the five-word table based at 0x801F4FBC that the dispatcher
 *           at 0x801F344C indexes with the work record byte at +2; the record
 *           is published at the scratchpad pointer cell 0x1F800044 and no jal
 *           in the payload reaches this span. It first records whether the
 *           signed halfword at 0x3A of that record was zero into byte 0x08 and
 *           takes the signed halfword at 0x36 as the index for this overlay's
 *           own seed tables: the byte at 0x801F4FE0 at twice that index goes
 *           into the halfword at 0x36, the byte at 0x801F4FE1 at that index
 *           into the halfword at 0x3A, the halfword at 0x801F4FD0 at that
 *           index into the halfword at 0x3E, the word at 0x801F4FD8 at that
 *           index into the word at 0x20 and the halfword at 0x801F4FD4 at
 *           that index into the halfword at 0x2E. It clears the halfword at
 *           0x30 and increments the state byte at 0x02 so the next dispatch
 *           takes entry 1 of the same table, then re-reads the route byte at
 *           0x08: when it is non-zero the window test takes the shared camera
 *           word 0x80145EC8 against the cell centre ((halfword at 0x3A plus 1)
 *           << 16) or 0x8000 and keeps 0x80145EC4 with the halfword at 0x36 as
 *           the shared tail operands, and when it is zero it takes 0x80145EC4
 *           against the centre of the halfword at 0x36 and keeps 0x80145EC8
 *           with the halfword at 0x3A instead. When that first distance is no
 *           further than 0x8000 and the kept camera word lies no further than
 *           0x10000 from the halfword's own cell start (halfword << 16) or from
 *           the next cell start ((halfword + 1) << 16), it stamps 0x80 into the
 *           record halfword at 0x30 and 3 into the state byte at 0x02. Every
 *           path finishes with the record renderer 0x801F3958. Takes no
 *           arguments and returns nothing; the 0x18-byte frame keeps $ra at
 *           0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTablesStampInside(
    void) {
  u8* work;
  u16 halfword2e;
  u8* gate;
  s32 index;
  s32 route;
  s32 value;
  s32 base;
  s32 limit;

  route = FIELD_REF(s16, D_1F800044, 0x3A) == 0;
  index = FIELD_REF(s16, D_1F800044, 0x36);
  FIELD_REF(u8, D_1F800044, 0x08) = route;

  work = D_1F800044;
  FIELD_REF(u16, work, 0x36) = D_801F4FE0[index * 2];
  FIELD_REF(u16, work, 0x3A) = D_801F4FE1[index * 2];
  FIELD_REF(u16, work, 0x3E) = D_801F4FD0[index];
  FIELD_REF(u32, work, 0x20) = D_801F4FD8[index];
  halfword2e = D_801F4FD4[index];
  ++FIELD_REF(u8, work, 0x02);
  gate = D_1F800044;
  FIELD_REF(u16, work, 0x30) = 0;
  FIELD_REF(u16, work, 0x2E) = halfword2e;

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
  renderWorkRecord();
}
