#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Record table at 0x801F09B0 in this overlay's trailing read-only block: four
 * bytes per record, 32 records per 128-byte row, the row pitch this body forms
 * as the sll 7 of work byte 0x4 and the record offset as the sll 2 of the record
 * index. A record is the halfword pair the band follows, so its two members stay
 * address-anchored and the base auto-binds through the generated
 * undefined-symbol table (D_801F09B0) with no map row of its own. */
typedef struct BmagicMagic17303PairRecord {
  u16 unk_00;
  u16 unk_02;
} BmagicMagic17303PairRecord;

extern BmagicMagic17303PairRecord D_801F09B0[][32]; /* @source 0x801F09B0 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The original re-materialises the address with a lui/lw pair
 * at each read (once for the entry row selector, once per loop step, once for
 * the loop bound), so the body never keeps the pointer in a local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad band cells at 0x1F800000-0x1F80000E, the eight halfwords this body
 * drives. The original addresses each cell absolutely before the loop (lui $at
 * plus the offset) and then forms ONE base register at 0x1F80000C for the loop,
 * which makes the cells below it negative displacements: the radius at -0xC, the
 * angle at -0xA, the turned angle at -0x8, the current point pair at -0x4 and
 * -0x2 and the prior point pair at 0x0 and 0x2. The loop re-reads the radius
 * cell 0x1F800000 from the separate base the radius store formed, so the cells
 * are declared one by one rather than as one record. The widths are the widths
 * the original picks: the radius with lh for the scale products, the angle with
 * lhu for the quarter turn, the turned angle with lh as the rotator argument,
 * the shade with lhu for the colour bytes and the four point cells with lhu for
 * the vertex sums and with lh for the heading difference, so those cells take an
 * explicit (s16) cast at the heading. Cell 0x1F800002 holds the heading of the
 * current record step, 0x1F800004 the same heading a quarter turn further on,
 * and 0x1F800006 the shade the two colour runs of one quad read back. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F80000E; /* @source 0x1F80000E @kind data */

/* Shared graphic-type query used for the two draw-mode page selectors. */
s32 GetGraphType(void);

/* Shared screen-heading helper at 0x8017A138: the 12-bit heading of the vector
 * (arg0, arg1). This body publishes its answer in the cell 0x1F800002 once as
 * the band heading and once per record step. */
s32 func_8017A138(s32 arg0, s32 arg1);

/* Shared rotator pair at 0x801782FC and 0x801783C8, the samples this body shifts
 * right 12 over the radius: the first for the x displacement and the second for
 * the y displacement of both vertex pairs. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801EF4EC
 * @behavior Gouraud band emitter of this overlay's byte-4 draw group. The payload's only reference to it is the jal at 0x801EEEA4
 *           inside func_801EEE3C, taken while the work object's byte 4 is
 *           non-zero, byte 0 is non-zero and byte 2 is non-zero; the body reads
 *           no incoming register, so it takes no arguments and returns nothing.
 *           It selects the draw-mode texture page 0xA5 when the shared graphic
 *           type is 1 or 2 and 0x35 otherwise, links a 0xC-byte draw-mode packet
 *           through SetDrawMode at g_PrimCursor with dfe 0 and dtd 1, and seeds
 *           the band radius cell 0x1F800000 with 2.
 *           The band runs along the record row of the pair table at 0x801F09B0
 *           that work byte 0x4 selects, from the first record at or after work
 *           byte 0xB whose leading halfword is not the 0xFFFF sentinel up to the
 *           record before work byte 0xA. The starting record is published as the
 *           prior point, the record after it as the current point, the heading of
 *           (current - prior) as the band heading, and the starting record again
 *           as the current point, so each pass then emits one quad per record
 *           step.
 *           Every corner is a record point pushed off the polyline by the
 *           rotator sample of one quarter turn (0x400 of the 0x1000 circle)
 *           scaled by the radius and shifted right 12: corners 0 and 2 sit on
 *           the prior point at heading + 0x400 and heading - 0x400, using the
 *           band heading the previous step published in cell 0x1F800002, and
 *           corners 1 and 3 sit on the current point at the same two turns of
 *           the heading re-derived from the published point cells, so the quad
 *           splits the record step into a left and a right side. Because the
 *           primitive stores may alias those cells the original re-reads them
 *           with a signed halfword load, which is why the heading arguments
 *           carry their own (s16) casts.
 *           Both colour runs of one quad read the scratch halfword 0x1F800006
 *           back after it is written, so the primitive stores force the reload
 *           the original shows: corners 0 and 2 take 0x61 - 12 (i - 1) and
 *           corners 1 and 3 take 0x61 - 12 i as their red and green, every blue
 *           byte is the fixed 1 and each quad is marked semi-transparent. Every
 *           vertex and colour write goes through a fresh g_PrimCursor POLY_G4
 *           load and each loop step ends by linking a 0x24-byte primitive. The
 *           body closes by selecting the draw-mode page 0x25 for graphic type 1
 *           or 2 and 0x15 otherwise and linking a 0xC-byte draw-mode packet at
 *           g_PrimCursor. The row selector and the loop bound are re-read after
 *           each link. The 0x40-byte frame holds $ra and all eight $s registers.
 * @status partial
 * @match 97.94
 * @residual The live comparison measured 332 of 339 instructions (97.94%) and
 *           1356 bytes against 1352. The first difference is at +0x1EC: the
 *           preheader copies i = i + 1 before the point-cell store rather than
 *           after the bound load, leaving an extra nop. The first shade scale
 *           shift also follows the entry branch instead of filling its delay
 *           slot. The band base and shade ramp now use the original registers;
 *           the loop body matches apart from relocated branch destinations.
 */
void drawTranslucentGouraudBandAlongRowRecordsFromWorkByteBToByteAOffsetByQuarterTurnRadiusTwoFadingRedGreenTwelveBmagicMagic17303_801EF4EC(
    void) {
  POLY_G4 *primitive;
  u16 *band;
  u16 *cell;
  s32 i;
  s32 graph;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(3, 0xC);

  i = g_battle_work[0xB];
  while ((s16)D_801F09B0[g_battle_work[4]][i].unk_00 == -1) {
    i++;
  }
  do {
    band = &D_1F800000;
  } while (0);
  *band = 2;
  cell = band + 6;
  D_1F80000C = D_801F09B0[g_battle_work[4]][i].unk_00;
  D_1F80000E = D_801F09B0[g_battle_work[4]][i].unk_02;
  D_1F800008 = D_801F09B0[g_battle_work[4]][i + 1].unk_00;
  D_1F80000A = D_801F09B0[g_battle_work[4]][i + 1].unk_02;
  D_1F800002 = func_8017A138((s16)D_1F800008 - (s16)D_1F80000C,
                             (s16)D_1F80000A - (s16)D_1F80000E);
  D_1F800008 = D_801F09B0[g_battle_work[4]][i].unk_00;
  D_1F80000A = D_801F09B0[g_battle_work[4]][i].unk_02;

  for (i = i + 1; i < g_battle_work[0xA]; i++) {
    cell[0] = cell[-2];
    cell[1] = cell[-1];
    cell[-2] = D_801F09B0[g_battle_work[4]][i].unk_00;
    cell[-1] = D_801F09B0[g_battle_work[4]][i].unk_02;
    primitive = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(primitive);
    SetSemiTrans(primitive, 1);

    ((s16 *)cell)[-4] = (cell[-5] + 0x400) & 0xFFF;
    primitive->x0 =
        cell[0] + ((func_801782FC(cell[-4]) * ((s16 *)cell)[-6]) >> 12);
    primitive->y0 =
        cell[1] + ((func_801783C8(((s16 *)cell)[-4]) * ((s16 *)cell)[-6]) >> 12);
    ((s16 *)cell)[-4] = (cell[-5] - 0x400) & 0xFFF;
    primitive->x2 =
        cell[0] + ((func_801782FC(cell[-4]) * ((s16 *)cell)[-6]) >> 12);
    primitive->y2 =
        cell[1] + ((func_801783C8(((s16 *)cell)[-4]) * ((s16 *)cell)[-6]) >> 12);

    cell[-5] = func_8017A138(((s16 *)cell)[-2] - ((s16 *)cell)[0],
                             ((s16 *)cell)[-1] - ((s16 *)cell)[1]);
    ((s16 *)cell)[-4] = (cell[-5] + 0x400) & 0xFFF;
    primitive->x1 =
        cell[-2] + ((func_801782FC(cell[-4]) * ((s16 *)cell)[-6]) >> 12);
    primitive->y1 =
        cell[-1] + ((func_801783C8(((s16 *)cell)[-4]) * ((s16 *)cell)[-6]) >> 12);
    ((s16 *)cell)[-4] = (cell[-5] - 0x400) & 0xFFF;
    primitive->x3 =
        cell[-2] + ((func_801782FC(cell[-4]) * ((s16 *)cell)[-6]) >> 12);
    primitive->y3 =
        cell[-1] + ((func_801783C8(((s16 *)cell)[-4]) * ((s16 *)cell)[-6]) >> 12);

    ((s16 *)band)[3] = 0x61 - 12 * (i - 1);
    primitive->r0 = band[3];
    primitive->g0 = band[3];
    primitive->b0 = 1;
    primitive->r2 = band[3];
    primitive->g2 = band[3];
    primitive->b2 = 1;
    ((s16 *)band)[3] = 0x61 - 12 * i;
    primitive->r1 = band[3];
    primitive->g1 = band[3];
    primitive->b1 = 1;
    primitive->r3 = band[3];
    primitive->g3 = band[3];
    primitive->b3 = 1;

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
