#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Record table of the trailing read-only block of this overlay at 0x801F0A38:
 * 0x80-byte rows, 32 four-byte records per row, the pitch this body forms as the
 * sll 7 of work byte 0x4 and the record offset as the sll 2 of the record index.
 * A record is the halfword pair the band follows, so its two members stay
 * address-anchored. The unlifted sites at 0x801EF4E0 and 0x801F02A0 still read
 * the same table from asm, so splat keeps binding the address through the
 * generated undefined-syms table and the target map needs no row of its own. */
typedef struct BmagicMagic04403PairRecord {
  u16 unk_00;
  u16 unk_02;
} BmagicMagic04403PairRecord;

extern BmagicMagic04403PairRecord D_801F0A38[][32]; /* @source 0x801F0A38 @kind data */

/* Scratchpad band record at 0x1F800000, the eight halfwords this body drives.
 * The original keeps ONE base register for the whole pair of loops and forms it
 * at the 0xC cell, so the cells below it become negative displacements of that
 * base and the two it addresses most (0x1F80000C and 0x1F80000E) are the
 * non-negative ones; the same block also gets one second base (addiu $s4, $s1,
 * -0xC) for the two cells that are cheaper from 0x0, the radius and the shade at
 * 0x6. Both bases address the same eight cells, so the record is one object and
 * the widths are the load widths the original picks: the radius with lh for the
 * scale products, the angle with lhu for the 0x400 turn, the turned angle with
 * lh as the rotator argument, the shade with lhu for the colour bytes and the
 * four point cells with lhu for the vertex sums and with lh for the heading
 * difference, so those two cells take an explicit (s16) cast at the heading.
 * The cells at 0x2, 0x4 and 0x6 are the band steering state: the heading of the
 * current record step, the same heading a quarter turn further on, and the shade
 * value the two colour runs of one quad read back. The point cells 0x8/0xA are
 * the record the loop is on and 0xC/0xE the record before it. */
typedef struct BmagicMagic04403BandScratch {
  s16 radius;     /* 0x0000 */
  u16 angle;      /* 0x0002 */
  s16 angleTurn;  /* 0x0004 */
  u16 shade;      /* 0x0006 */
  u16 pointX;     /* 0x0008 */
  u16 pointY;     /* 0x000A */
  u16 priorX;     /* 0x000C */
  u16 priorY;     /* 0x000E */
} BmagicMagic04403BandScratch;

extern BmagicMagic04403BandScratch D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. This body never keeps the pointer in a local: every row
 * selector and every loop test re-reads the cell, because the table loads and
 * the primitive stores between the reads may alias it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

s32 GetGraphType(void);

/* Shared screen-heading helper at 0x8017A138: the 12-bit heading of the vector
 * (arg0, arg1). This body publishes its answer in the steering cell 0x1F800002
 * once per band pass and once per record step. */
s32 func_8017A138(s32 arg0, s32 arg1);

/* Shared rotator pair at 0x801782FC and 0x801783C8, the samples the two bodies of
 * this family shift right 12 over the published radius. This body takes the
 * 0x801782FC sample for the x displacement and the 0x801783C8 sample for the y
 * displacement of both vertex pairs. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801EFA4C
 * @behavior Gouraud band emitter of this overlay's work-byte-2 handler group.
 *           Its only in-image reference is the jal at 0x801EEEB0 inside
 *           dispatchWorkByte2HandlerThenDrawByWorkByte4BmagicMagic04403_801EEE44,
 *           taken while the work object's byte 0 is non-zero, byte 2 is non-zero
 *           and byte 4 is zero; the body reads no incoming register, so it takes
 *           no arguments and returns nothing.
 *           It selects the draw-mode texture page 0xA5 when the shared graphic
 *           type is 1 or 2 and 0x35 otherwise, links a 0xC-byte draw-mode packet
 *           through SetDrawMode at g_PrimCursor with dfe 0 and dtd 1, and seeds
 *           the band radius cell 0x1F800000 with 0xC.
 *           The band runs along the record row of the pair table at 0x801F0A38
 *           that work byte 0x4 selects, from the first record at or after work
 *           byte 0xB whose leading halfword is not the 0xFFFF sentinel up to the
 *           record before work byte 0xA. The starting record is published as the
 *           prior point, the record after it as the current point, the heading of
 *           (current - prior) as the band heading, and the starting record again
 *           as the current point, so each pass then emits one quad per record
 *           step.
 *           A quad has the corners prior+offset, current+offset, prior and
 *           current, where the offset is the rotator sample of the band heading
 *           turned a quarter turn (0x400 of the 0x1000 circle), scaled by the
 *           radius and shifted right 12. The heading is re-derived from the
 *           published point cells before each vertex pair, so the offset follows
 *           the record polyline; because the primitive stores may alias those
 *           cells the original re-reads them with a signed halfword load, which is
 *           why the heading arguments carry their own (s16) casts. Both passes start from the same record and emit the same
 *           number of quads, drawing the band on the two sides of the polyline:
 *           the first pass turns the heading towards the left and gives
 *           corners 0 and 2 the shade 0x61 - 6 (i - 1) and corners 1 and 3
 *           0x61 - 6 i, the second turns it towards the right and gives the same
 *           corners 0x81 - 8 (i - 1) and 0x81 - 8 i.
 *           The shade is published in the scratch halfword 0x1F800006 and read
 *           back for the red and blue byte of its corner, so the primitive stores
 *           force the reload the original shows; every green byte is the fixed 1
 *           and each quad is marked semi-transparent. Every vertex and colour
 *           write goes through a fresh g_PrimCursor POLY_G4 load, each pass ends
 *           by linking a 0x24-byte primitive, and the body closes by selecting the
 *           draw-mode page 0x25 for graphic type 1 or 2 and 0x15 otherwise and
 *           linking a 0xC-byte draw-mode packet at g_PrimCursor. The row selector,
 *           the loop bound and the band cells are re-read after each link. The
 *           0x40-byte frame holds $ra and all eight $s registers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudBandAlongRowRecordsFromWorkByteBToByteAOffsetByQuarterTurnRadiusTwelveBmagicMagic04403_801EFA4C(
    void) {
  POLY_G4 *primitive;
  s32 i;
  s32 graph;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);

  D_1F800000.radius = 0xC;
  i = g_battle_work[0xB];
  while ((s16)D_801F0A38[g_battle_work[4]][i].unk_00 == -1) {
    i++;
  }
  D_1F800000.priorX = D_801F0A38[g_battle_work[4]][i].unk_00;
  D_1F800000.priorY = D_801F0A38[g_battle_work[4]][i].unk_02;
  D_1F800000.pointX = D_801F0A38[g_battle_work[4]][i + 1].unk_00;
  D_1F800000.pointY = D_801F0A38[g_battle_work[4]][i + 1].unk_02;
  D_1F800000.angle = func_8017A138(
      (s16)D_1F800000.pointX - (s16)D_1F800000.priorX,
      (s16)D_1F800000.pointY - (s16)D_1F800000.priorY);
  D_1F800000.pointX = D_801F0A38[g_battle_work[4]][i].unk_00;
  D_1F800000.pointY = D_801F0A38[g_battle_work[4]][i].unk_02;

  for (i = i + 1; i < g_battle_work[0xA]; i++) {
    D_1F800000.priorX = D_1F800000.pointX;
    D_1F800000.priorY = D_1F800000.pointY;
    D_1F800000.pointX = D_801F0A38[g_battle_work[4]][i].unk_00;
    D_1F800000.pointY = D_801F0A38[g_battle_work[4]][i].unk_02;
    primitive = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(primitive);
    SetSemiTrans(primitive, 1);

    D_1F800000.angleTurn = (D_1F800000.angle + 0x400) & 0xFFF;
    primitive->x0 =
        D_1F800000.priorX +
        ((func_801782FC(D_1F800000.angleTurn) * D_1F800000.radius) >> 12);
    primitive->y0 =
        D_1F800000.priorY +
        ((func_801783C8(D_1F800000.angleTurn) * D_1F800000.radius) >> 12);
    primitive->x2 = D_1F800000.priorX;
    primitive->y2 = D_1F800000.priorY;

    D_1F800000.angle = func_8017A138(
        (s16)D_1F800000.pointX - (s16)D_1F800000.priorX,
        (s16)D_1F800000.pointY - (s16)D_1F800000.priorY);
    D_1F800000.angleTurn = (D_1F800000.angle + 0x400) & 0xFFF;
    primitive->x1 =
        D_1F800000.pointX +
        ((func_801782FC(D_1F800000.angleTurn) * D_1F800000.radius) >> 12);
    primitive->y1 =
        D_1F800000.pointY +
        ((func_801783C8(D_1F800000.angleTurn) * D_1F800000.radius) >> 12);
    primitive->x3 = D_1F800000.pointX;
    primitive->y3 = D_1F800000.pointY;

    D_1F800000.shade = 0x61 - 6 * (i - 1);
    primitive->r0 = 1;
    primitive->g0 = 1;
    primitive->b0 = 1;
    primitive->r2 = D_1F800000.shade;
    primitive->g2 = 1;
    primitive->b2 = D_1F800000.shade;
    D_1F800000.shade = 0x61 - 6 * i;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r3 = D_1F800000.shade;
    primitive->g3 = 1;
    primitive->b3 = D_1F800000.shade;

    GpuAppendPrim(3, 0x24);
  }

  i = g_battle_work[0xB];
  while ((s16)D_801F0A38[g_battle_work[4]][i].unk_00 == -1) {
    i++;
  }
  D_1F800000.priorX = D_801F0A38[g_battle_work[4]][i].unk_00;
  D_1F800000.priorY = D_801F0A38[g_battle_work[4]][i].unk_02;
  D_1F800000.pointX = D_801F0A38[g_battle_work[4]][i + 1].unk_00;
  D_1F800000.pointY = D_801F0A38[g_battle_work[4]][i + 1].unk_02;
  D_1F800000.angle = func_8017A138(
      (s16)D_1F800000.pointX - (s16)D_1F800000.priorX,
      (s16)D_1F800000.pointY - (s16)D_1F800000.priorY);
  D_1F800000.pointX = D_801F0A38[g_battle_work[4]][i].unk_00;
  D_1F800000.pointY = D_801F0A38[g_battle_work[4]][i].unk_02;

  for (i = i + 1; i < g_battle_work[0xA]; i++) {
    D_1F800000.priorX = D_1F800000.pointX;
    D_1F800000.priorY = D_1F800000.pointY;
    D_1F800000.pointX = D_801F0A38[g_battle_work[4]][i].unk_00;
    D_1F800000.pointY = D_801F0A38[g_battle_work[4]][i].unk_02;
    primitive = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(primitive);
    SetSemiTrans(primitive, 1);

    D_1F800000.angleTurn = (D_1F800000.angle - 0x400) & 0xFFF;
    primitive->x0 =
        D_1F800000.priorX +
        ((func_801782FC(D_1F800000.angleTurn) * D_1F800000.radius) >> 12);
    primitive->y0 =
        D_1F800000.priorY +
        ((func_801783C8(D_1F800000.angleTurn) * D_1F800000.radius) >> 12);
    primitive->x2 = D_1F800000.priorX;
    primitive->y2 = D_1F800000.priorY;

    D_1F800000.angle = func_8017A138(
        (s16)D_1F800000.pointX - (s16)D_1F800000.priorX,
        (s16)D_1F800000.pointY - (s16)D_1F800000.priorY);
    D_1F800000.angleTurn = (D_1F800000.angle - 0x400) & 0xFFF;
    primitive->x1 =
        D_1F800000.pointX +
        ((func_801782FC(D_1F800000.angleTurn) * D_1F800000.radius) >> 12);
    primitive->y1 =
        D_1F800000.pointY +
        ((func_801783C8(D_1F800000.angleTurn) * D_1F800000.radius) >> 12);
    primitive->x3 = D_1F800000.pointX;
    primitive->y3 = D_1F800000.pointY;

    D_1F800000.shade = 0x81 - 8 * (i - 1);
    primitive->r0 = 1;
    primitive->g0 = 1;
    primitive->b0 = 1;
    primitive->r2 = D_1F800000.shade;
    primitive->g2 = 1;
    primitive->b2 = D_1F800000.shade;
    D_1F800000.shade = 0x81 - 8 * i;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r3 = D_1F800000.shade;
    primitive->g3 = 1;
    primitive->b3 = D_1F800000.shade;

    GpuAppendPrim(3, 0x24);
  }

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);
}
