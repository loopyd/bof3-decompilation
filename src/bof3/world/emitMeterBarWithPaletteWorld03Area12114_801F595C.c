#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* @source 0x801F2C04 @kind table */
extern u8 D_801F2C04[];

/* One palette entry of the meter bar texture at 0x801F2C04: the high nibble of
 * `clut` selects the sub-palette inside the 0x7800 clut block and `u` is the
 * texture column of the bar's left edge. The two entries are copied into a
 * local table because the original reads them through a byte array. */
typedef struct {
  u8 clut;
  u8 u;
} MeterBarCell;

typedef struct {
  MeterBarCell cell[2];
} MeterBarPalette;

/* @source 0x801F595C
 * @behavior Appends one textured FT4 bar to the primitive cursor at 0x8014598C.
 * The quad runs from (0xE4, 0x1D) to (0xE4 + (width & 0xFF), 0x1D + 0xA) and is
 * flat 0x80 grey. Its texture is the 4x8 texel cell on texture page 0xF chosen
 * by the 2-entry palette table at 0x801F2C04: the (palette & 0xFF) entry's low
 * byte is the left texture column and that byte plus 4 the right one, while the
 * entry's high nibble becomes clut bits 0-5 above the 0x7800 base. The local
 * GetGraphType probe is kept as the original wrote it, and the finished
 * primitive is handed to func_8014E5A0 at 0x28 bytes. Takes a width and a
 * palette index and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The palette lookup is the first field written after tpage, so the table entry
 * address is computed while the x/y block is still being stored, and the four u
 * coordinates are settled before the v/rgb block: measured 75/81 at 328 bytes
 * with the clut write after y1 and the u1/u2/u3 writes after the rgb stores,
 * 81/81 at 324 bytes with the two groups in this order. */
void emitMeterBarWithPaletteWorld03Area12114_801F595C(s32 width, s32 palette) {
  POLY_FT4* prim;
  MeterBarPalette cells;

  prim = (POLY_FT4*)g_PrimCursor;
  cells = *(const MeterBarPalette*)D_801F2C04;
  SetPolyFT4(prim);
  if (GetGraphType() != 1) {
    GetGraphType();
  }
  prim->tpage = 0xF;
  prim->clut = (cells.cell[(u8)palette].clut >> 4) | 0x7800;
  prim->x0 = 0xE4;
  prim->x1 = (u8)width + 0xE4;
  prim->y0 = 0x1D;
  prim->y1 = 0x1D;
  prim->x2 = prim->x0;
  prim->x3 = prim->x1;
  prim->y2 = prim->y0 + 0xA;
  prim->y3 = prim->y2;
  prim->u0 = cells.cell[(u8)palette].u;
  prim->u1 = prim->u0 + 4;
  prim->u2 = prim->u0;
  prim->u3 = prim->u1;
  prim->v0 = 0xD8;
  prim->v1 = 0xD8;
  prim->r0 = 0x80;
  prim->g0 = 0x80;
  prim->b0 = 0x80;
  prim->v2 = prim->v0 + 8;
  prim->v3 = prim->v2;
  func_8014E5A0(2, 0x28);
}
