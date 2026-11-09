#include "bof3/bof3.h"
#include "gpu/prim.h"

/* The overlay's own six-entry tile table, the head of the textbin row that
 * starts at D_801F0B64. Every entry is four bytes: the texel-cell origin of one
 * 12x12 cell, its u halfword at +0 and its v halfword at +2. The original walks
 * two halfword pointers four bytes apart, the u pointer one element below the v
 * pointer, and reads each uv store's halfword at zero displacement from them.
 * @source 0x801F0B98 @kind table */
extern u8 D_801F0B98[];

/* @source 0x801F0B9A @kind table */
extern u8 D_801F0B9A[];

/* One past the last tile entry: the word that opens the pointer run at
 * D_801F0BB0. Both loop tests compare the v pointer against this address, so
 * the run of six cells ends here.
 * @source 0x801F0BB2 @kind data */
extern u8 D_801F0BB2[];

/* The shared battle selector byte. Its value minus three indexes the 0x118-byte
 * battle-target record whose own flag bytes select this row's cell tint, the
 * same record family the exact sibling
 * queueSlotPerEligibleEnemyAndPartyBattlerSetByte1To2OrdinalByte4SelectorByteBAndRecordPointer
 * BmagicMagic06903_801F0274 walks at its own 0x118 stride.
 * @source 0x80146384 @kind unknown */
extern u8 D_80146384;

/* @source 0x801EB6DF @kind data */
extern u8 D_801EB6DF[];

/* Scratchpad cell this body uses as its tint scratch word: the 0x30/0x80 grey
 * it stores there is re-read once per colour channel. It is a word cell, so the
 * three channel stores each reload it.
 * @source 0x1F800004 @kind unknown */
extern u32 D_1F800004;

/* Matching note: the uv corner stores read the tile table through two halfword
 * pointer locals, the u pointer derived from the v pointer, so every access is a
 * zero-displacement lhu/lbu off a walked base. Naming the table indices as a
 * subscript instead makes gcc hoist the symbol address into two saved registers
 * and grow the frame, which the magic08000 sibling
 * drawSixTileTexturedRowAtWorkHalfwords2EAnd30FromTileTableBmagicMagic08000_801EED94
 * records for the same six-cell row. */
/* @source 0x801EFCE8
 * @behavior Draws one row of six 12x12 texel cells at the screen pair (x, y).
 *   The row starts at x and advances x by 0x14 per cell, so the cells overlap by
 *   four units; y is fixed for the whole row.
 *   Each cell is a 0x28-byte POLY_FT4 taken from the primitive cursor
 *   g_PrimCursor and stamped by SetPolyFT4: x0 and x2 take x, x1 and x3 take
 *   x + 0xC, y0 and y1 take y, y2 and y3 take y + 0xC. The texture page is
 *   GetTPage(0, 1, 0x3C0, 0x100) and the clut GetClut(0, 0x1E0).
 *   The four uv corners come from the overlay tile table at D_801F0B98, walked
 *   by two halfword pointers four bytes apart: u0 and u2 read the entry's u
 *   halfword, u1 and u3 read that halfword's low byte plus 0xC, while v0 and v1
 *   read the entry's v halfword and v2 and v3 read that halfword's low byte plus
 *   0xC. The six entries are the 0x18-byte run that ends at D_801F0BB2; the
 *   table stores interleave with the pointer reads, so every uv store re-reads
 *   the table.
 *   The cell tint is set before the colour stores: the body stores the grey 0x30
 *   in the scratchpad word 0x1F800004 and, for a cell whose own flag byte is
 *   below the arm's threshold, overwrites it with the grey 0x80. The flag byte
 *   is chosen by the cell number through a six-arm switch over the record at
 *   D_801EB6DF + (D_80146384 - 3) * 0x118: arms 0, 1 and 2 take that record's
 *   bytes +0, +1 and +2 and test them below 2, arm 3 takes byte +4 and arm 4
 *   byte +3, both below 2, and arm 5 takes byte +5 below 4. The three channels
 *   then each reload the scratchpad word, so r0, g0 and b0 all take that grey.
 *   The cell is queued with GpuAppendPrim(2, 0x28) after its uv stores. Nothing
 *   is returned and no state outside the primitive cursor and the scratchpad
 *   word is written. The 0x38-byte frame keeps $ra at 0x30 and $s0-$s7 at
 *   0x10-0x2C. Every literal (six cells, 0xC, 0x14, 0x30, 0x80, 0x100,
 *   0x1E0, 0x3C0, 0x28, 0x118, 2, 3, 4) is this body's own, read from the
 *   original bytes at 0x801EFCE8.
 * @status partial
 * @match 64.71
 * @residual The reference switch carries no bound check of its own: the only
 *   test before the table load is the enclosing unsigned pointer guard, whose
 *   four instructions this source reproduces exactly. Written here, gcc keeps
 *   the switch's own casesi check (sltiu/beqz over the cell counter) because it
 *   cannot prove the counter from the pointer bound, so this body is two
 *   instructions and eight bytes over at 612 for the original 604, and the
 *   prologue's saved-register order differs from the original's as a
 *   consequence: the x argument lands in $s5 rather than $s4 and the first
 *   difference is the third stack store. The uv/colour/texture body, the six
 *   switch arms, the guard and the loop test match.
 */
void drawSixTileTexturedRowAtScreenPairFromTileTableTintedBySelectorRecordFlagsBmagicMagic06000_801EFCE8(
    s32 x, s32 y) {
  POLY_FT4 *prim;
  u16 *pu;
  u16 *pv;
  u32 i;
  u32 on;
  u32 offset;

  i = 0;
  offset = (D_80146384 - 3) * 0x118;
  pv = (u16 *)D_801F0B9A;
  pu = pv - 1;
  do {
    prim = (POLY_FT4 *)g_PrimCursor;
    SetPolyFT4(prim);
    prim->x0 = x;
    prim->y0 = y;
    prim->x1 = x + 0xC;
    prim->y1 = y;
    prim->x2 = x;
    prim->y2 = y + 0xC;
    prim->x3 = x + 0xC;
    prim->y3 = y + 0xC;
    D_1F800004 = 0x30;
    if ((u32)pv < 0x801F0BB2u) {
      switch (i) {
        case 0:
          on = D_801EB6DF[offset] < 2u;
          break;
        case 1:
          on = D_801EB6DF[offset + 1] < 2u;
          break;
        case 2:
          on = D_801EB6DF[offset + 2] < 2u;
          break;
        case 3:
          on = D_801EB6DF[offset + 4] < 2u;
          break;
        case 4:
          on = D_801EB6DF[offset + 3] < 2u;
          break;
        case 5:
          on = D_801EB6DF[offset + 5] < 4u;
          break;
      }
      if (on != 0u) {
        D_1F800004 = 0x80;
      }
    }
    prim->r0 = D_1F800004;
    prim->g0 = D_1F800004;
    prim->b0 = D_1F800004;
    prim->tpage = GetTPage(0, 1, 0x3C0, 0x100);
    prim->clut = GetClut(0, 0x1E0);
    prim->u0 = *pu;
    prim->v0 = *pv;
    prim->u1 = *(u8 *)pu + 0xC;
    prim->v1 = *pv;
    prim->u2 = *pu;
    prim->v2 = *(u8 *)pv + 0xC;
    prim->u3 = *(u8 *)pu + 0xC;
    prim->v3 = *(u8 *)pv + 0xC;
    GpuAppendPrim(2, 0x28);
    i++;
    x += 0x14;
    pu += 2;
    pv += 2;
  } while ((s32)pv < (s32)D_801F0BB2);
}
