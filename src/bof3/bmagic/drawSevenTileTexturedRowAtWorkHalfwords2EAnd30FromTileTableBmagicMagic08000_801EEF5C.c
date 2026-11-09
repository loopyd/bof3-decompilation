#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map.
 * The original materialises the cell once per iteration with its own
 * lui 0x1F80 / lw 0x44 pair after the SetPolyFT4 call and keeps that base in
 * $v1 across all seven halfword reads of 0x2E and 0x30, so the cell is named
 * rather than written as a bare scratchpad constant. The exact same-target
 * siblings seedWorkChannelPairDistanceRandomByteBCountdownAndAdvanceWorkByte1
 * BmagicMagic08000_801EEC5C and decayWorkHalfword2EThenDrawByWorkByteBAndRaise
 * Flag4WhenByte9ExpiresBmagicMagic08000_801EECE4 declare the same cell the same
 * way. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The overlay's tile table, the head of this payload's textbin row at
 * 0x801EF124. Every entry is four bytes: the (u, v) halfword pair of one 12x12
 * texel cell sits at +0 and +2, and the original reads each halfword at a byte
 * offset of the loop counter times four, so the run is a byte table of
 * 4-byte entries. This body reads the interior word 0x801EF13C, six entries
 * beyond the head that the six-tile row variant
 * drawSixTileTexturedRowAtWorkHalfwords2EAnd30FromTileTableBmagicMagic08000_801EED94
 * reads, the same way, so the run keeps its address-qualified name and one
 * splat row. */
extern u8 D_801EF13C[]; /* @source 0x801EF13C @kind unknown */

/* Matching note: the scaled table offset belongs in a separate `offset` local.
 * The exact six-tile sibling measured the stride inline in the subscript at
 * 82/114 instructions and 424 bytes, because gcc then hoists the whole symbol
 * address into two saved registers and the frame grows to 0x30 bytes, so the
 * product stays in its own local here and the original folded form
 * `lui $at,%hi(D_801EF13C); addu $at,$at,$v1; lhu/lbu %lo($at)` with $v1
 * holding the counter times four has one place to come from. */
/* @source 0x801EEF5C
 * @behavior Non-zero-mask textured row variant of the MAGIC080 overlay: the
 *   draw entry the work byte 1 state 1 handler func_801EECE4 selects when the
 *   random frame-mask byte 0x0B is non-zero, where the same handler selects the
 *   six-tile row variant
 *   drawSixTileTexturedRowAtWorkHalfwords2EAnd30FromTileTableBmagicMagic08000_801EED94
 *   for a zero mask. It emits seven 0x28-byte POLY_FT4 primitives from the
 *   cursor cell 0x8014598C, one per loop iteration, each after SetPolyFT4
 *   stamps it. Every primitive is a 12x12 texel cell: x0 and x2 take the work
 *   halfword 0x2E plus the counter times 0xC and x1 and x3 take that halfword
 *   plus the counter plus one times 0xC, so the seven cells tile a 0x54-unit
 *   horizontal row, while y0 and y1 take the work halfword 0x30 and y2 and y3
 *   take it plus 0xC. The row is drawn with 0x80 in all three colour channels,
 *   the texture page GetTPage(0, 1, 0x3C0, 0) and the clut GetClut(0, 0x1E0).
 *   The four uv corners come from the overlay tile table 0x801EF124 at its
 *   interior word 0x801EF13C, six entries beyond the head, indexed by the
 *   counter: u0, u1 and u2 read the entry halfword at +0 and u3 that halfword
 *   plus 0xC, v0, v1 and v2 read the entry halfword at +2 and v3 that halfword
 *   plus 0xC, so each primitive covers one 12x12 cell of the page at the entry
 *   cell origin and the original store order is u0, v0, u1, v1, u2, v2, u3, v3.
 *   Every uv store re-reads the table because the prim stores may alias it, and
 *   the row is queued with GpuAppendPrim(2, 0x28) after its uv stores. It takes
 *   no argument, returns nothing, and the 0x28-byte frame only keeps $ra plus
 *   $s0-$s3 across the five calls per iteration. Every literal (seven, 0xC,
 *   0x80, 0x3C0, 0, 0x1E0, 0x28, 2) is this body's own, read from the original
 *   bytes at 0x801EEF5C. Statement order, operand forms and declaration style
 *   follow the exact same-target six-tile sibling, and the ten instruction
 *   words the two bodies do not share are that sibling's loop bound, its uv
 *   table head and its GetTPage last argument alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSevenTileTexturedRowAtWorkHalfwords2EAnd30FromTileTableBmagicMagic08000_801EEF5C(
    void) {
  POLY_FT4 *prim;
  u8 *work;
  s32 i;
  u32 offset;

  for (i = 0; i < 7; i++) {
    prim = (POLY_FT4 *)g_PrimCursor;
    SetPolyFT4(prim);
    work = g_battle_work;
    prim->x0 = *(u16 *)(work + 0x2E) + i * 0xC;
    prim->y0 = *(u16 *)(work + 0x30);
    prim->x1 = *(u16 *)(work + 0x2E) + (i + 1) * 0xC;
    prim->y1 = *(u16 *)(work + 0x30);
    prim->x2 = *(u16 *)(work + 0x2E) + i * 0xC;
    prim->y2 = *(u16 *)(work + 0x30) + 0xC;
    prim->x3 = *(u16 *)(work + 0x2E) + (i + 1) * 0xC;
    prim->y3 = *(u16 *)(work + 0x30) + 0xC;
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    prim->tpage = GetTPage(0, 1, 0x3C0, 0);
    prim->clut = GetClut(0, 0x1E0);
    offset = i * 4;
    prim->u0 = *(u16 *)(D_801EF13C + offset);
    prim->v0 = *(u16 *)(D_801EF13C + offset + 2);
    prim->u1 = D_801EF13C[offset] + 0xC;
    prim->v1 = *(u16 *)(D_801EF13C + offset + 2);
    prim->u2 = *(u16 *)(D_801EF13C + offset);
    prim->v2 = D_801EF13C[offset + 2] + 0xC;
    prim->u3 = D_801EF13C[offset] + 0xC;
    prim->v3 = D_801EF13C[offset + 2] + 0xC;
    GpuAppendPrim(2, 0x28);
  }
}
