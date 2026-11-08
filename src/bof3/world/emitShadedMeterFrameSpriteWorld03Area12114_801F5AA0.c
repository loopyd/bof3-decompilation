#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* @source 0x801F5AA0
 * @behavior Emits the fixed 0x50-by-0x20 shade sprite of the AREA121 meter frame
 * that its only caller at 0x801F5B90 places at (0xDC, 0x10). The draw-mode
 * texture page word is 0x22C when the graph type reads 1 or 2 and 0x9C
 * otherwise; that DR_MODE is sized 0xC and appended to the primitive cursor at
 * 0x8014598C under ordering-table index 2 before the sprite itself. The SPRT is
 * fixed at 0x50 by 0x20 with CLUT 0x7B80, texture column 0 and texture row
 * ((shade & 0xFF) << 5) + 0x78, so the shade argument selects one 0x20-tall row
 * band of that texture, and it is drawn at (x, y) in flat 0x80 grey and appended
 * at 0x14 bytes under ordering-table index 2. Takes an x, a y and a shade index
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The shade band is kept in its own local because the original publishes the
 * zero-extended byte before the draw-mode call and reads the row back from that
 * register: the same row written inline as `((shade & 0xFF) << 5) + 0x78` let
 * the byte store swallow the mask and measured 58/60 at 236 bytes, while the
 * `(u8)shade` local keeps the original `andi` at 60/60. The field writes keep
 * the source order x/y/w/h/clut/u0/row, which the scheduler then retimes into
 * the original w/h/clut/row/x/y/u0/rgb sequence. */
void emitShadedMeterFrameSpriteWorld03Area12114_801F5AA0(s32 x, s32 y, s32 shade) {
  SPRT* prim;
  s32 tpage;
  s32 row;

  if (GetGraphType() == 1) {
    tpage = 0x22c;
  } else if (GetGraphType() == 2) {
    tpage = 0x22c;
  } else {
    tpage = 0x9c;
  }

  row = (u8)shade;
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  func_8014E5A0(2, 0xc);

  prim = (SPRT*)g_PrimCursor;
  SetSprt(prim);
  prim->x0 = x;
  prim->y0 = y;
  prim->w = 0x50;
  prim->h = 0x20;
  prim->clut = 0x7b80;
  prim->u0 = 0;
  prim->v0 = (row << 5) + 0x78;
  setRGB0(prim, 0x80, 0x80, 0x80);
  func_8014E5A0(2, 0x14);
}
