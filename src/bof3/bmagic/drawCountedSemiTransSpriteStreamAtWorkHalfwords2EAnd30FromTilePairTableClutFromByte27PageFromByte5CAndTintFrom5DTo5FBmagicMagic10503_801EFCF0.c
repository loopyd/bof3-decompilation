#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The record walker func_801EEC14 repoints the cell at one of
 * the 64 records of the 0x78-byte pool at 0x801F00A0 before it runs the record
 * state handler and this body, then restores the saved cell, so every read here
 * sees that one record. Each read re-materialises the cell with its own
 * lui 0x1F80 / lw 0x44 pair because a store through the primitive cursor may
 * alias the cell; the leading culling block is the only run of reads with no
 * store between them and therefore shares one load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared graphic type report at 0x8017B2B4. The body asks it twice inside one
 * expression to pick the texture-page bits of the draw-mode word. */
extern s32 GetGraphType(void); /* @source 0x8017B2B4 */

/* Overlay tile-size table at 0x801F0080, the interior word of the payload data
 * span that the target map keeps as the single textbin row D_801F0048. Every
 * entry is a byte pair and the body reads the pair of entry (record byte 0 and
 * 0xF): the byte at 0x801F0080 becomes the sprite width and the byte at
 * 0x801F0081, one past it, its height, so the run is a byte table of two-byte
 * entries read as two adjacent interior words. */
extern u8 D_801F0080[]; /* @source 0x801F0080 @kind unknown */
extern u8 D_801F0081[]; /* @source 0x801F0081 @kind unknown */

/* @source 0x801EFCF0
 * @behavior Sprite pass of the record walker func_801EEC14: the body it calls
 *   through the jal at 0x801EECD0 after the record state handler
 *   dispatchStateByte1Table0074BmagicMagic10503_801EF880, with the scratchpad
 *   work-object cell 0x1F800044 repointed at the record under test, so the work
 *   object it draws is one record of the 0x78-byte pool at 0x801F00A0.
 *   It takes no argument and returns nothing. Three culling tests guard the
 *   draw: it returns when work byte 0 has bit 0x40 set, when the work halfword
 *   0x2E plus 0x40 reads 0x1C1 or more as u16, and when the work halfword 0x30
 *   plus 0x40 reads 0x171 or more, so a record is drawn only while its centre
 *   lies within those screen bounds.
 *   It then takes the record sprite stream from the work pointer word 0x54
 *   advanced by the work halfword 0x5A: its head byte is the record count and
 *   the records that follow are five bytes each, the byte at +0 holding the tile
 *   index in the low nibble and the mirror flag 0x80, +1 and +2 the signed y and
 *   x offsets, +3 the texture u coordinate and +4 the v coordinate. It computes
 *   the clut word once for the whole stream: (work byte 0x27 + 0x1E0) << 6 when
 *   work byte 0x28 is non-zero, otherwise ((byte 0x27 >> 4) + 0x1F0) << 6 or-ed
 *   with the low nibble of byte 0x27. A zero count returns before the loop.
 *   Each iteration first selects the draw-mode texture page: when GetGraphType
 *   reads 1 or 2 it is ((work byte 0x5C and 3) << 7) or-ed with 0x22D for a
 *   non-zero work byte 0x28 and 0x2D otherwise; for every other graph type it is
 *   ((byte 0x5C and 3) << 5) or-ed with 0x9D and 0x1D the same way. That page
 *   word goes to SetDrawMode at the primitive cursor with dfe and dtd zero, then
 *   the cursor is appended at 0x8014E5A0 with work byte 0x29 as the ordering
 *   index and size 0xC.
 *   It takes the next cursor SPRT, stamps it with SetSprt, and fills it: width
 *   and height are the two bytes of tile-size entry (record byte 0 and 0xF),
 *   u0 is record +3, v0 is record +4 plus work byte 0x26, clut is the word
 *   computed above, and the three colours are work bytes 0x5D, 0x5E and 0x5F
 *   each taken minus 0x80.
 *   x0 is the work halfword 0x2E plus the signed record byte +2, or, when the
 *   record carries flag 0x80 and work byte 0x2A is non-zero, that halfword minus
 *   the signed record byte +2 minus the width just stored, which mirrors the
 *   sprite about the centre. y0 is the work halfword 0x30 plus the signed record
 *   byte +1 on both paths. The record walk and the tile counter advance by five
 *   bytes, the primitive is flagged semi-transparent with bit 5 of work byte 0,
 *   and the cursor is appended again with work byte 0x29 and size 0x14 before
 *   the count is decremented and the loop repeats.
 *   Every literal quoted above (0x40, 0x1C1, 0x171, 0x2E, 0x30, 0x54, 0x5A,
 *   0x27, 0x28, 0x1E0, 0x1F0, 0x5C, 0x22D, 0x2D, 0x9D, 0x1D, 0x29, 0xC, 0x26,
 *   0x5D, 0x5E, 0x5F, 0x80, 0x2A, 5, 0x14) is this body's own, read from the
 *   original bytes at 0x801EFCF0. The body walks one record cursor; the original
 *   keeps two pointers five bytes apart and two bytes apart from each other, and
 *   this shape reproduces that split from the single cursor.
 * @status partial
 * @match 99.53
 * @residual Same-size residual (856/856 bytes and the same instruction multiset,
 *   213 of 214 instructions aligned): the three remaining differences are
 *   instruction order inside two basic blocks - the draw-mode argument setup
 *   emits the dtd zero after the g_PrimCursor load instead of before it, and the
 *   loop tail emits the count decrement before the semi-transparency argument
 *   load instead of after the trailing cursor reload. Eighteen clean-C shape
 *   sweeps over more than fifty variants (single versus paired table symbols,
 *   indexed versus pointer-arithmetic table access, (s8) cast versus shift sign
 *   extension, duplicated versus shared y0, one versus two record cursors, the
 *   count decrement as statement versus loop condition, page and argument
 *   permutations, declaration order) all measured this same pair of hunks.
 *   Assembler profile, compiler flags and permuter stay parent-owned.
 */
void drawCountedSemiTransSpriteStreamAtWorkHalfwords2EAnd30FromTilePairTableClutFromByte27PageFromByte5CAndTintFrom5DTo5FBmagicMagic10503_801EFCF0(
    void) {
  SPRT *prim;
  u8 *stream;
  u8 *record;
  u8 count;
  s32 page;
  s32 clut;
  s32 tint;

  if (g_battle_work[0] & 0x40) {
    return;
  }
  if ((u16)(*(u16 *)(g_battle_work + 0x2E) + 0x40) >= 0x1C1) {
    return;
  }
  if ((u16)(*(u16 *)(g_battle_work + 0x30) + 0x40) >= 0x171) {
    return;
  }
  stream = (u8 *)(*(u32 *)(g_battle_work + 0x54) + *(u16 *)(g_battle_work + 0x5A));
  count = stream[0];
  record = stream + 1;
  if (g_battle_work[0x28] != 0) {
    clut = (g_battle_work[0x27] + 0x1E0) << 6;
  } else {
    clut = (((g_battle_work[0x27] >> 4) + 0x1F0) << 6) | (g_battle_work[0x27] & 0xF);
  }
  if (count == 0) {
    return;
  }
  do {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      page = ((g_battle_work[0x5C] & 3) << 7) |
             (g_battle_work[0x28] ? 0x22D : 0x2D);
    } else {
      page = ((g_battle_work[0x5C] & 3) << 5) |
             (g_battle_work[0x28] ? 0x9D : 0x1D);
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, page, 0);
    GpuAppendPrim(g_battle_work[0x29], 0xC);

    prim = (SPRT *)g_PrimCursor;
    SetSprt(prim);
    prim->w = D_801F0080[(record[0] & 0xF) * 2];
    prim->h = D_801F0081[(record[0] & 0xF) * 2];
    prim->u0 = record[3];
    prim->v0 = g_battle_work[0x26] + record[4];
    prim->clut = clut;
    tint = g_battle_work[0x5D] - 0x80;
    prim->r0 = tint;
    tint = g_battle_work[0x5E] - 0x80;
    prim->g0 = tint;
    tint = g_battle_work[0x5F] - 0x80;
    prim->b0 = tint;
    if ((record[0] & 0x80) && g_battle_work[0x2A] != 0) {
      prim->x0 = *(u16 *)(g_battle_work + 0x2E) - (s8)record[1] - prim->w;
      prim->y0 = *(u16 *)(g_battle_work + 0x30) + (s8)record[2];
    } else {
      prim->x0 = *(u16 *)(g_battle_work + 0x2E) + (s8)record[1];
      prim->y0 = *(u16 *)(g_battle_work + 0x30) + (s8)record[2];
    }
    SetSemiTrans(prim, (g_battle_work[0] >> 5) & 1);
    GpuAppendPrim(g_battle_work[0x29], 0x14);
    record += 5;
    count--;
  } while (count != 0);
}
