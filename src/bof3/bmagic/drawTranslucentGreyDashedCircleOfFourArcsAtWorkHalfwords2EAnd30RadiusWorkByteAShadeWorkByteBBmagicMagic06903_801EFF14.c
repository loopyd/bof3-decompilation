#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The body materialises the cell before the frame is created,
 * keeps the pointer in $v1 across the four seed reads and then reaches the work
 * fields 0x02, 0x0A and 0x0B through it with zero-base displacements. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad ring block at 0x1F800000: the signed radius cell 0x00 the body
 * publishes from work byte 0x0A, the unused cell 0x02, the segment-angle cell
 * 0x04, the grey-level cell 0x06, the unused cells 0x08 and 0x0A, and the centre
 * pair 0x0C/0x0E seeded from work halfwords 0x2E and 0x30. The loop reaches all
 * of them through the one base register the original forms after the seed
 * publishes (lui 0x1F80, addiu 4), so the angle sits at the base and the other
 * cells at the displacements -4, 2, 8 and 0xA the original's loads and stores
 * show. The radius is published as an unsigned halfword and consumed with a
 * signed load, the angle is stored as a halfword and read back with a signed
 * load for the second rotator argument of each corner, and the centre pair and
 * the shade are read with unsigned loads, so the member widths below are the
 * load widths the original picks. The angle is published with the assignment
 * inside each rotator argument, which is the shape that leaves the stores in
 * the two call delay slots. */
typedef struct BmagicMagic06903RingScratch {
  s16 radius;   /* 0x0000 */
  u16 unk_02;   /* 0x0002 */
  s16 angle;    /* 0x0004 */
  u16 shade;    /* 0x0006 */
  u16 unk_08;   /* 0x0008 */
  u16 unk_0A;   /* 0x000A */
  u16 centreX;  /* 0x000C */
  u16 centreY;  /* 0x000E */
} BmagicMagic06903RingScratch;

extern BmagicMagic06903RingScratch D_1F800000; /* @source 0x1F800000 @kind data */

/* Main-executable primitive initialiser at 0x8017AA80, the helper this overlay's
 * sibling lift at 0x801F0104 also calls before it writes a flat line primitive.
 * It takes the packet pointer and returns nothing. */
void func_8017AA80(void *primitive);

/* Shared rotator pair at 0x801782FC and 0x801783C8: the samples this body shifts
 * right 12 over the published signed radius, the first for the x displacement of
 * a point and the second for its y displacement. Both take the 16-bit angle as a
 * sign-extended scalar and return the scaled step. */
s32 func_801782FC(s32 angle);
s32 func_801783C8(s32 angle);

/* @source 0x801EFF14
 * @behavior Ring emitter of the MAGIC069 work object. Its only in-image
 *    references are the jal at 0x801EEF88 inside this overlay's per-frame draw
 *    function func_801EEED4 and the jal at 0x801EF3C0 inside the state-7 frame
 *    function func_801EF30C; both guard it with work byte 0 bit 0 set and work
 *    byte 2 non-zero, both follow it with the sibling draw routine func_801EFC00
 *    and both run it between the two draw-mode packets of the frame. It reads no
 *    incoming register, so it takes no arguments and returns nothing, and the
 *    0x28-byte frame keeps $ra at 0x24 and $s0..$s4 at 0x10..0x20 ($s0 the
 *    primitive cursor, $s1 the scratchpad base, $s2 the phase index, $s3 the next
 *    segment angle and $s4 the arc counter).
 *    The four seed publishes write the scratchpad block the ring is drawn from:
 *    the radius cell 0x1F800000 takes work byte 0x0A, the centre cell 0x1F80000C
 *    takes work halfword 0x2E, the centre cell 0x1F80000E takes work halfword
 *    0x30, and the grey-level cell 0x1F800006 takes 0x80 - 0x30 * work byte 0x0B
 *    while work byte 0x02 is below 2 and the bare 0x80 otherwise. All four are
 *    absolute halfword stores formed with their own lui 0x1F80 (the original
 *    uses $at and never keeps the address), which is why they are written through
 *    the scratchpad addresses rather than through the block the loop uses.
 *    The loop then walks one full turn in 0x40 steps of the 0x1000 angle circle,
 *    that is 64 candidate segment angles, and emits a segment only for the arc
 *    counter values whose (arc & 0xC) is 4 or 8, so it draws the two middle
 *    quarters of each group of four and skips the other two. The drawn values
 *    4..11, 20..27, 36..43 and 52..59 form four 45-degree arcs that start 22.5
 *    degrees after the axes, each followed by a 45-degree gap, so the ring is a
 *    dashed circle: grey (equal red, green and blue), semi-transparent, centred
 *    on the cached work screen pair and sized by the work radius byte.
 *    A segment is one semi-transparent flat 0x10-byte primitive taken from
 *    g_PrimCursor, initialised by func_8017AA80 and SetSemiTrans with 1 before
 *    its fields are written and queued with GpuAppendPrim(3, 0x10). Its two
 *    corners are the scratchpad centre plus the rotator sample of a segment angle
 *    times the signed radius shifted right 12, in the statement order x0, y0, x1,
 *    y1; the first corner uses the phase angle i * 0x40 and the second the next
 *    angle, which the body keeps in its own accumulator that advances by 0x40 in
 *    the loop latch, while the second rotator argument of each corner re-reads
 *    the published halfword with a signed load. Each of the three colour bytes is
 *    a fresh read of the grey-level cell.
 * @status partial
 * @match 99.19
 * @residual 123/124 instructions at the original 496-byte image, 496 bytes
 *   emitted, so the only difference is the position of one instruction. The
 *   original initialises the next-angle accumulator (li $s3, 0x40) after the
 *   scratchpad base is formed in the loop preheader, which is where a loop-created
 *   induction variable's initial value would land, while this candidate keeps
 *   that accumulator as a source variable and so emits its initialisation one
 *   statement earlier, before the phase index and the base. Every instruction of
 *   the seed publishes, the arc test, both corners of the primitive, the three
 *   colour bytes, the queue call, the register assignment ($s0..$s4 in the
 *   original's roles) and the loop latch match. Source shapes measured for that
 *   one placement: the accumulator initialised in the for-init list, before the
 *   phase index, from a declaration initialiser, through a comma expression, with
 *   the index initialised before the loop, and with the store value instead
 *   spelled as the phase expression (i * 0x40, (i + 1) * 0x40, i * 0x40 + 0x40,
 *   with and without (u16)/(s16) casts and with the store inside the rotator
 *   argument) — the expression spellings either drop the accumulator to 107-118
 *   of 124 with a smaller frame or move the value off $s3, so the source-variable
 *   spelling above is the retained one.
 */
void drawTranslucentGreyDashedCircleOfFourArcsAtWorkHalfwords2EAnd30RadiusWorkByteAShadeWorkByteBBmagicMagic06903_801EFF14(
    void) {
  LINE_F2 *primitive;
  u8 *work;
  s32 i;
  s32 phase;
  s32 next;
  s32 step;
  u16 shade;

  work = g_battle_work;
  SPAD_REF(u16, 0x00) = FIELD_REF(u8, work, 0x0A);
  SPAD_REF(u16, 0x0C) = FIELD_REF(u16, work, 0x2E);
  SPAD_REF(u16, 0x0E) = FIELD_REF(u16, work, 0x30);
  if (FIELD_REF(u8, work, 0x02) < 2) {
    shade = 0x80 - FIELD_REF(u8, work, 0x0B) * 0x30;
  } else {
    shade = 0x80;
  }
  SPAD_REF(u16, 0x06) = shade;
  phase = 0;
  next = 0x40;
  for (i = 0; i < 0x40; i++) {
    if ((phase & 0xC) == 4 || (phase & 0xC) == 8) {
      primitive = (LINE_F2 *)g_PrimCursor;
      func_8017AA80(primitive);
      SetSemiTrans(primitive, 1);
      D_1F800000.angle = i * 0x40;
      step = func_801782FC((s16)(i * 0x40));
      primitive->x0 = D_1F800000.centreX + ((step * D_1F800000.radius) >> 12);
      step = func_801783C8(D_1F800000.angle);
      primitive->y0 = D_1F800000.centreY + ((step * D_1F800000.radius) >> 12);
      D_1F800000.angle = next;
      step = func_801782FC((s16)((i + 1) * 0x40));
      primitive->x1 = D_1F800000.centreX + ((step * D_1F800000.radius) >> 12);
      step = func_801783C8(D_1F800000.angle);
      primitive->y1 = D_1F800000.centreY + ((step * D_1F800000.radius) >> 12);
      FIELD_REF(u8, primitive, 4) = D_1F800000.shade;
      FIELD_REF(u8, primitive, 5) = D_1F800000.shade;
      FIELD_REF(u8, primitive, 6) = D_1F800000.shade;
      GpuAppendPrim(3, 0x10);
    }
    next += 0x40;
    phase++;
  }
}
