#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell. The original materialises the cell with
 * its own lui/lw pair and keeps the record pointer live across the three
 * palette products, so it is read once into a local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared sampler pair of a 12-bit turn. 0x801782FC feeds every x halfword and
 * 0x801783C8 every y halfword of the ring, and the original shifts those
 * results with `srl`, so both keep the unsigned return. */
extern u32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern u32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */

/* Shared graphic-type query that names the two draw-mode pages. */
extern s32 GetGraphType(void);

/* Scratchpad halfword cells the body keeps as the live base of a pointer walk.
 * The original forms each address with a `lui %hi` plus `addiu %lo` pair, the
 * symbol form rather than a folded literal, seeds the colour triple through the
 * 0x1F800008 pointer, then copies that address into the saved register the loop
 * reads the triple through, and re-bases the ring from the inner cell by minus
 * eight bytes. */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */

/* The three channel rows of the palette block that starts at 0x801F0EE0 in
 * this payload's trailing read-only subsegment. The original masks each row
 * with its own %hi/%lo pair over bases 0x801F0EE0, 0x801F0EE1 and 0x801F0EE2,
 * so the three channels are separate symbols, and the three-byte stride is what
 * selects the entry. */
extern u8 D_801F0EE0[]; /* @source 0x801F0EE0 @kind table */
extern u8 D_801F0EE1[]; /* @source 0x801F0EE1 @kind table */
extern u8 D_801F0EE2[]; /* @source 0x801F0EE2 @kind table */

/* @source 0x801EFB24
 * @behavior Translucent ring draw step of this overlay's work-byte-2 handler
 *           group. Its only in-image reference is the jal at 0x801EF1B4 inside
 *           the unlifted 0x801EF1DC, and the body reads no incoming register,
 *           so it takes no arguments and returns nothing.
 *           It asks GetGraphType twice and takes the draw-mode page 0xA5 when
 *           either answer is 1 or 2 and 0x35 otherwise, links that page through
 *           SetDrawMode at the prim cursor, and appends the 0xC-byte draw-mode
 *           packet under slot 5. It then reads the scratchpad work object once:
 *           the object's byte 0x04 selects the palette entry, scaled by three,
 *           and byte 0x09 scales the entry, and the three channel products are
 *           published as the halfword colour triple at 0x1F800008, 0x1F80000A
 *           and 0x1F80000C.
 *           The ring is seeded from the sampler pair at phase 0: the inner
 *           vertex 0x1F80001C and 0x1F80001E take the sampler results shifted
 *           right five, which places them at radius 128, and the outer vertex
 *           0x1F80002C and 0x1F80002E take results shifted right four, the
 *           doubled radius 256. Sixteen slices follow, one per sixteenth of the
 *           turn: each moves the previous inner pair into vertex 0 and the
 *           previous outer pair into vertex 2, reseeds vertices 1 and 3 from the
 *           samplers at phase i shifted left eight, clears the four depth
 *           halfwords from the last vertex back, links a 0x24-byte POLY_G4
 *           packet, projects the four scratchpad vertices with RotTransPers4,
 *           tints vertices 0 and 1 from the published colour triple and
 *           vertices 2 and 3 with the constant one, then appends that packet
 *           under slot 5. The body closes by restoring the draw-mode page,
 *           taking 0x25 for graph types 1 and 2 and 0x15 otherwise, and
 *           appending the closing 0xC-byte packet under the same slot.
 *
 *           Shape evidence, not clone proof: a masked scan of all extracted
 *           payloads under out/binaries found this 0x330-byte span only at its
 *           own offset, and the one near twin at magic082 index 3 offset 0xE00
 *           differs in exactly the three palette row loads, each by the same
 *           0x1D8, which is a carried data immediate. The clone lever is
 *           therefore ineligible and the statement order, the operand forms and
 *           the mixed pointer/folded scratchpad spellings are transcribed from
 *           the exact family siblings at magic167 index 3 address 0x801EF4B0
 *           (the sixteen-slice fan, its page pair and its sampler shifts) and
 *           magic039 index 3 address 0x801EFAA4 (the palette row selection).
 *           The page constants, the sixteen-slice bound, the sampler shifts and
 *           the palette rows are this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudQuadRingOfSixteenQuadsSteppingSixteenthTurnBetweenRadius128And256TintedByPaletteRowAtWorkByte4ScaledByWorkByte9BmagicMagic08303_801EFB24(
    void) {
  u8 *work;
  u16 *tint;
  u16 *shade;
  u16 *inner;
  SVECTOR *ring;
  POLY_G4 *prim;
  long depth;
  long flag;
  s32 graph;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(5, 0xC);

  work = g_battle_work;
  shade = &D_1F800008;
  *shade = D_801F0EE0[work[4] * 3] * work[9];
  SPAD_REF(u16, 0x0Au) = D_801F0EE1[work[4] * 3] * work[9];
  SPAD_REF(u16, 0x0Cu) = D_801F0EE2[work[4] * 3] * work[9];

  inner = &D_1F80001C;
  *inner = func_801782FC(0) >> 5;
  SPAD_REF(u16, 0x1Eu) = func_801783C8(0) >> 5;
  SPAD_REF(u16, 0x2Cu) = func_801782FC(0) >> 4;
  SPAD_REF(u16, 0x2Eu) = func_801783C8(0) >> 4;

  i = 1;
  ring = (SVECTOR *)(inner - 4);
  tint = shade;
  do {
    phase = i << 8;
    ring[0].vx = ring[1].vx;
    ring[0].vy = ring[1].vy;
    ring[2].vx = ring[3].vx;
    ring[2].vy = ring[3].vy;
    ring[1].vx = func_801782FC(phase) >> 5;
    ring[1].vy = func_801783C8(phase) >> 5;
    ring[3].vx = func_801782FC(phase) >> 4;
    ring[3].vy = func_801783C8(phase) >> 4;
    prim = (POLY_G4 *)g_PrimCursor;
    ring[3].vz = 0;
    ring[2].vz = 0;
    ring[1].vz = 0;
    ring[0].vz = 0;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    RotTransPers4(ring, ring + 1, ring + 2, ring + 3, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                  &depth, &flag);
    prim->r0 = tint[0];
    prim->g0 = tint[1];
    prim->b0 = tint[2];
    prim->r1 = tint[0];
    prim->g1 = tint[1];
    prim->b1 = tint[2];
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    prim->r3 = 1;
    prim->g3 = 1;
    prim->b3 = 1;
    GpuAppendPrim(5, 0x24);
    i++;
  } while (i < 0x11);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(5, 0xC);
}
