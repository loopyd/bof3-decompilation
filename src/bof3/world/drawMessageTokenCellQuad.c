#include "bof3/world/area03004_internal.h"

/* Graphics-type query of the running executable, owned by the SDK map at
 * 0x8017B2B4. This payload only calls it. */
extern s32 GetGraphType(void);

/* Shared panel origin halfword: the shipped body draws the message cell row
 * three pixels below it. */
extern u16 D_80144154; /* @source 0x80144154 @kind unknown */

/* @source 0x801D9740
 * @behavior draws one panel message token as a single flat textured quad from
 * the shared primitive cursor. It stamps a POLY_FT4, selects the texture page
 * from the graphics type (0x8F when the query answers 1 or 2 and 0x2F
 * otherwise, replaced by 0xAF or 0x3F on the table-cell path), and derives the
 * atlas cell of the token byte: the ASCII cell of (byte - 0x30) on the 31-column
 * table with an 8-pixel stride and a row offset of 0x48, the cell of
 * (byte + 0x5B) for control code 0x15, or, for control codes 0x12 and 0x13, the
 * cell of the byte the code carries (plus 0x100 for 0x13) on the 21-column
 * table with a 12-pixel stride. Both cell formulas keep the column remainder
 * times the pixel stride as the horizontal texture coordinate and the row
 * quotient times 12 as the vertical one. The quad spans the requested x to
 * x + (byte 0x7F mask) at the two rows D_80144154 + 3 and + 0xC, takes the mid
 * grey 0x80 as its colour, the CLUT row 0x1E0 with the low nibble of the first
 * argument, and, when bit 0x80 of the width byte is set, moves the horizontal
 * texture coordinate 8 - width to the right so the quad shows the tail of the
 * cell. The 0x28-byte packet is appended at ordering-table slot 1.
 * @status partial
 * @match 91.84
 * @residual the arms, the cell arithmetic, the quad stores and the append
 *           reproduce the original's 245 instructions and 980 bytes, but the
 *           `prim->tpage` store of the 0x15 and default arms keeps the slot
 *           before the following division's `lui`, where the original takes it
 *           after, and the scheduler walks the tail store group in another order
 *           (the width mask and the right-corner sum land in $a1/$v1 against
 *           $v1/$v0). First difference: index 37 of 245.
 */
void drawMessageTokenCellQuad(u8 arg0, u8* arg1, u8 arg2, s16 arg3) {
  POLY_FT4* prim;
  u8 cellRight;

  prim = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(prim);

  switch (*arg1) {
  case 0x15:
    arg1 += 1;
    if (GetGraphType() == 1) {
      prim->tpage = 0x8F;
    } else if (GetGraphType() == 2) {
      prim->tpage = 0x8F;
    } else {
      prim->tpage = 0x2F;
    }
    prim->u0 = (*arg1 + 0x5B) % 21 * 12;
    prim->v0 = (*arg1 + 0x5B) / 21 * 12;
    break;

  case 0x12:
  case 0x13:
    if (GetGraphType() == 1) {
      prim->tpage = 0xAF;
    } else if (GetGraphType() == 2) {
      prim->tpage = 0xAF;
    } else {
      prim->tpage = 0x3F;
    }
    if (*arg1 == 0x12) {
      arg1 += 1;
      prim->u0 = (u32)*arg1 % 21 * 12;
      prim->v0 = (u32)*arg1 / 21 * 12;
    } else {
      arg1 += 1;
      prim->u0 = (*arg1 + 0x100) % 21 * 12;
      prim->v0 = (*arg1 + 0x100) / 21 * 12;
    }
    break;

  default:
    if (GetGraphType() == 1) {
      prim->tpage = 0x8F;
    } else if (GetGraphType() == 2) {
      prim->tpage = 0x8F;
    } else {
      prim->tpage = 0x2F;
    }
    prim->u0 = (*arg1 - 0x30) % 31 * 8;
    prim->v0 = (*arg1 - 0x30) / 31 * 12 + 0x48;
    break;
  }

  if (arg2 & 0x80) {
    cellRight = prim->u0 + 8;
    prim->u0 = cellRight - (arg2 & 0x7F);
  }

  prim->x0 = arg3;
  prim->x1 = arg3 + (arg2 & 0x7F);
  prim->u1 = prim->u0 + (arg2 & 0x7F);
  prim->v1 = prim->v0;
  prim->u2 = prim->u0;
  prim->v3 = prim->v2 = prim->v0 + 12;
  prim->x2 = prim->x0;
  prim->x3 = prim->x1;
  prim->u3 = prim->u1;
  prim->r0 = 0x80;
  prim->g0 = 0x80;
  prim->b0 = 0x80;
  prim->clut = (arg0 & 0xF) | 0x7800;
  prim->y0 = D_80144154 + 3;
  prim->y1 = prim->y0;
  prim->y3 = prim->y2 = prim->y0 + 12;
  GpuAppendPrim(1, 0x28);
}
