#include "bof3/bof3.h"

/* Shared flag tester of the main executable, the same probe the sibling
 * handlers of this target call with a flag-bank base and a flag number. */
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Shared UI element queue of the main executable. The constant-only callers of
 * this target declare it with signed halfword arguments; the coordinates here
 * are computed, and the original forwards them without narrowing, so this
 * declaration keeps the full width. */
void func_801A4BC0(s32 x, s32 y, u32 size);

/* Shared cell texture pair of the main executable: 0x801558F0 resolves the
 * record of the map cell at (x, y) and 0x80155560 copies that record's texture
 * state into the caller's cell entry. */
s32 func_801558F0(s32 x, s32 y);
void func_80155560(u32 arg0, void* arg1, s32 arg2);

/* Flag bank of the main executable, probed here at flag 0x7E. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Map work block of the main executable. Its first byte is the row stride (read
 * here through the block base, which the original materialises as a full 32-bit
 * constant), the next byte the row count, and the halfword at offset 2 an entry
 * bias that both the cell table below and the cell entry array are indexed
 * with. */
extern u8 D_80104001;  /* @source 0x80104001 @kind data */
extern u16 D_80104002; /* @source 0x80104002 @kind data */

/* Per-cell halfword table of the map work block, read at the cell's row-major
 * index scaled by two and biased by four times the bias field; the value it
 * holds selects the cell's entry. The row stride makes the true element type a
 * halfword, so the table is declared as the byte view the original loads from. */
extern u8 D_8010402C[]; /* @source 0x8010402C @kind table */

/* Per-cell record table the resolved cell index points into, indexed here by
 * its 0x50-byte stride. */
extern u8 D_8012D880[]; /* @source 0x8012D880 @kind table */

/* The overlay banner tables: 0x801F57CC holds two 7x9 code planes, 0x801F584C
 * one 7x9 size plane, and 0x801F58D0 the three 7x9 glyphs the banner draws; one
 * byte per cell in each. 0x801F5990 starts the three per-glyph row phases. The
 * plane dimensions are declared because the original's address chains add the
 * symbol first and each index after it, which is what a subscript chain
 * produces. */
extern u8 D_801F57CC[0x2][0x7][0x9]; /* @source 0x801F57CC @kind table */
extern u8 D_801F584C[0x7][0x9];      /* @source 0x801F584C @kind table */
extern u8 D_801F58D0[0x3][0x7][0x9]; /* @source 0x801F58D0 @kind table */
extern u8 D_801F5990[];              /* @source 0x801F5990 @kind table */

/* @source 0x801F3994
 * @behavior Stamps the 7x9 cell grid of the overlay banner into the map and
 *           queues one shared UI element per cell. Flag 0x7E of the shared flag
 *           bank 0x80144F28 is probed once through func_8015B5D4; while it is
 *           clear, the cell value is the union of the three 0x3F-byte glyphs of
 *           the banner, each read at row*9 plus its own row phase from
 *           0x801F5990 and wrapped to zero once the phase reaches nine, folded
 *           to one for any set cell and zero when all three are clear. While
 *           the flag is set the cell value stays zero.
 *           The cell then queues func_801A4BC0 at (col + 0x16, row + 0x8E) with
 *           the size byte D_801F584C[row][col] scaled by the cell value, so an
 *           uncovered cell queues at size zero. Its map entry is the word of the
 *           entry array that follows the block's per-cell halfword area: the
 *           block base plus ((stride * rowCount + 1) >> 1) * 4, plus four times
 *           the bias field, plus four times the block's cell halfword
 *           D_8010402C[bias*4 + (row + 0x8E)*stride*2 + col*2]. The upper three
 *           bytes of that word are kept and its low byte takes the code byte
 *           D_801F57CC[cellValue][row][col], written as a masked store followed
 *           by the code store. Finally func_801558F0 resolves the same
 *           (col + 0x16, row + 0x8E) cell and, when it returns a record, that
 *           record's 0x50-byte stride entry of D_8012D880 is attached to the
 *           entry word through func_80155560 with kind 2. Takes no arguments and
 *           returns nothing; the 0x40-byte frame keeps $ra and the eight saved
 *           registers, and the probed flag byte is spilled once and reloaded per
 *           cell.
 * @status partial
 * @match 98.64
 * @residual 145 of 147 instructions match at the original 596-byte size.
 *           Two commutative address additions have reversed operands: the
 *           scaled row offset plus the bias, and the cell-table base plus its
 *           byte offset. All other instructions match.
 *
 * Matching shape: the three banner tables are indexed as subscript chains
 * (plane[first][row][cell]), which is what makes the original add the symbol to
 * the first index and then each further index separately; a flat
 * one-dimensional subscript folds the symbol into the load displacement and
 * drops the plane sizes. The 63-byte cell value is accumulated in a u8 so the
 * mask is emitted once at the merge and again at the store, and the map block
 * base is materialised as a full 32-bit constant while the block's own fields
 * are read through their D_ symbols.
 */
void stampGlyphGridCellCodesQueueCellElementsAndAttachCellTextureWorld04Area17013_801F3994(
    void) {
  u8 flagClear;
  u8 row;
  u8 col;
  u8 glyph;
  u8 cell;
  u8* mapWork;
  u32* entry;
  s32 record;
  u8 phase;
  u32 biasBase;
  u32 cellOffset;
  u32 areaBase;

  flagClear = func_8015B5D4((u32)&D_80144F28, 0x7E) == 0;

  for (row = 0; row < 0x7; row++) {
    for (col = 0; col < 0x9; col++) {
      mapWork = PSX_PTR(u8, 0x80104000);
      cell = 0;
      if (flagClear) {
        for (glyph = 0; glyph < 0x3; glyph++) {
          phase = D_801F5990[glyph] + col;
          if (phase >= 0x9) {
            phase -= 0x9;
          }
          cell += D_801F58D0[glyph][row][phase];
        }
        if (cell != 0) {
          cell = 1;
        }
      }

      func_801A4BC0(col + 0x16, row + 0x8E, D_801F584C[row][col] * cell);

      areaBase = ((mapWork[0] * D_80104001 + 1) >> 1) * 4 + (u32)mapWork;
      biasBase = D_80104002 * 4;
      cellOffset = (row + 0x8E) * mapWork[0] * 2 + biasBase + col * 2;
      biasBase += areaBase;
      entry = (u32*)(biasBase + *(u16*)(D_8010402C + cellOffset) * 4);
      *entry &= 0xFFFFFF00;
      *entry |= D_801F57CC[cell][row][col];

      record = func_801558F0(col + 0x16, row + 0x8E);
      if (record != 0) {
        func_80155560(*entry, D_8012D880 + record * 0x50, 2);
      }
    }
  }
}
