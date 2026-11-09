#include "bof3/bof3.h"

/* Published scratchpad work object: the original materialises this cell with a
 * lui plus an offset load, so the cell keeps its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Primitive cursor cell of the shared main executable. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Shared frame counter of the main executable; only its low nibble is read
 * here, as the phase that scales the work object's origin vector. */
extern s32 D_80143E6C;

/* Primitive submit helper of the shared main executable: it takes the ordering
 * table slot and the primitive size as its two byte arguments. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* Segment stamper of the shared main executable, called on the fresh cursor
 * primitive before each trail pass writes its own fields. */
void func_8017AA30(void* arg0);

/* Point projector of the shared main executable: it projects the three-word
 * point handed in arg0 into the screen entry handed in arg1. */
void func_801AFF04(const void* arg0, void* arg1);

/* @source 0x801D1678
 * @behavior Draws the beam body between the caller's head point and trail point
 * and then its stepped trail. The 0x24-byte POLY_G4 published at g_PrimCursor
 * is stamped through SetPolyG4, marked semi-transparent with one and filled
 * from two projections through func_801AFF04: vertices 0 and 1 take the head
 * point pair widened by (+1,-1) and that pair plain, vertices 2 and 3 take the
 * trail point pair widened by (+1,-1) and that pair plain, the first and third colour
 * triples stay black while the second and fourth carry the intensity byte in
 * red and green with blue zero, and func_8014E5A0 appends the packet under OT
 * slot 2 at its 0x24-byte size. A second quad repeats that stamp on the freshly
 * reloaded cursor, projects the head point for vertices 0 and 1, which take the
 * plain pair and that pair widened by (-1,+1), and the trail point for vertices
 * 2 and 3, which take the plain pair and that pair widened by (-1,+1); its
 * first and third triples carry the intensity byte in red and green and its
 * second and fourth stay black, and the packet is appended the same way. The
 * stepped trail then starts at the trail point plus the work object's origin
 * vector at +0x64/+0x68/+0x6C scaled by the low nibble of the shared frame
 * counter 0x80143E6C and, while the head point's first word stays below the
 * marching x, stamps each pass through func_8017AA30, marks it semi-transparent
 * with one, writes the projected pair at +0x08/+0x0A and the intensity byte at
 * +0x04/+0x05 with blue zero, appends the 0xC-byte segment under OT slot 2 and
 * advances all three marching words by the origin vector shifted left four. It
 * takes the head point, the trail point and the tint byte and returns nothing;
 * the 0x50-byte frame keeps $ra, $s0 and $s2-$s4.
 *
 * Name evidence: the only in-image references to 0x801D1678 are the three
 * step-handler calls 0x801D106C, 0x801D12A0 and 0x801D13F4, which hand it the
 * work object head point at +0x18, the point at +0x34 and a tint byte, and the
 * body draws the beam body quads between exactly those two projected points
 * before stepping the trail from the second one until the first one's x is
 * passed, so the module stays the overlay scenario directory and the role is
 * the beam sliver pair plus its stepped trail.
 *
 * Evidence: the 0x24 packet size, the SetPolyG4/SetSemiTrans pair and the
 * +0x08/+0x0A/+0x10/+0x12/+0x18/+0x1A/+0x20/+0x22 vertex stores with the
 * triples at +0x04/+0x0C/+0x14/+0x1C fix both packets as POLY_G4; the two
 * projections per packet are the two argument triples, which the callers pass
 * as the work object words at +0x18 and +0x34, and the widened pairs are one
 * screen unit apart, which is why every offset is a plain plus or minus one.
 * The 0xC-byte segment the trail appends is the shape the exact sibling
 * drawTintedPairOfGouraudSliversAndEightSteppedSegmentsBetweenProjectedPointsScenarioSce15ef000_801D1C1C
 * of the same overlay family records for func_8017AA30: projected pair at
 * +0x08/+0x0A and the three colour bytes at +0x04/+0x05/+0x06. The origin words
 * at +0x64/+0x68/+0x6C are the object triple this overlay's exact lifts
 * func_801D0DD8 and func_801D0F10 copy into the step point at +0x34, and the
 * three mult-then-store pairs here are the VECTOR step spelling the exact
 * sibling above compiles to.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Levers: the data lever is declined because this span opens with the
 * `addiu $sp, $sp, -0x50` prologue, ends with `jr $ra` and holds fourteen `jal`
 * sites, and because `bin/harness analysis data-candidates` ranks no range of
 * this target at all; the carve lever is declined because the span starts at
 * its own declared entry 0x801D1678 and ends at the next declared entry
 * 0x801D199C with no shared epilogue; the clone lever is declined because no
 * payload under out/binaries holds this 0x324-byte span, the plain scan and the
 * jal-masked scan both miss, and every one of the fourteen call targets
 * resolves in the shared main executable, so no carried call word can differ
 * between overlays and the miss is conclusive rather than a relocation effect.
 * The body was seeded directly from the original bytes with the sibling shapes
 * above as spelling evidence.
 */
void drawBeamSliverPairBetweenHeadAndTrailPointThenSteppedTrailSegmentsWhileHeadXLagsStepXScenarioSce15ef100_801D1678(
    u8* from, u8* to, u8 intensity) {
  POLY_G4* prim;
  VECTOR current;
  VECTOR step;
  u16 projected[2];
  u8* segment;
  s32 phase;

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  func_801AFF04(from, projected);
  prim->x0 = projected[0] + 1;
  prim->y0 = projected[1] - 1;
  prim->x1 = projected[0];
  prim->y1 = projected[1];
  func_801AFF04(to, projected);
  prim->x2 = projected[0] + 1;
  prim->y2 = projected[1] - 1;
  prim->x3 = projected[0];
  prim->y3 = projected[1];
  prim->r0 = 0;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->r1 = intensity;
  prim->g1 = intensity;
  prim->b1 = 0;
  prim->r2 = 0;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = intensity;
  prim->g3 = intensity;
  prim->b3 = 0;
  func_8014E5A0(2, 0x24);

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 1);
  func_801AFF04(from, projected);
  prim->x0 = projected[0];
  prim->y0 = projected[1];
  prim->x1 = projected[0] - 1;
  prim->y1 = projected[1] + 1;
  func_801AFF04(to, projected);
  prim->x2 = projected[0];
  prim->y2 = projected[1];
  prim->x3 = projected[0] - 1;
  prim->y3 = projected[1] + 1;
  prim->r0 = intensity;
  prim->g0 = intensity;
  prim->b0 = 0;
  prim->r1 = 0;
  prim->g1 = 0;
  prim->b1 = 0;
  prim->r2 = intensity;
  prim->g2 = intensity;
  prim->b2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0;
  func_8014E5A0(2, 0x24);

  step.vx = *(s32*)(D_1F800044 + 0x64);
  step.vy = *(s32*)(D_1F800044 + 0x68);
  step.vz = *(s32*)(D_1F800044 + 0x6C);
  phase = D_80143E6C & 0xF;
  current.vx = *(s32*)(to + 0x00) + step.vx * phase;
  current.vy = *(s32*)(to + 0x04) + step.vy * phase;
  current.vz = *(s32*)(to + 0x08) + step.vz * phase;
  while (*(s32*)(from + 0x00) < current.vx) {
    func_801AFF04(&current, projected);
    segment = g_PrimCursor;
    func_8017AA30(segment);
    SetSemiTrans(segment, 1);
    *(u16*)(segment + 0x08) = projected[0];
    *(u16*)(segment + 0x0A) = projected[1];
    segment[4] = intensity;
    segment[5] = intensity;
    segment[6] = 0;
    func_8014E5A0(2, 0xC);
    current.vx += *(s32*)(D_1F800044 + 0x64) << 4;
    current.vy += *(s32*)(D_1F800044 + 0x68) << 4;
    current.vz += *(s32*)(D_1F800044 + 0x6C) << 4;
  }
}
