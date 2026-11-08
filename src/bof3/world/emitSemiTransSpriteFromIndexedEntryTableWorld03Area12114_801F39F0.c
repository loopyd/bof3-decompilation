#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* @source 0x801F6800 @kind table */
extern u8 D_801F6800[];

/* @source 0x801F39F0
 * @behavior Emits one semi-transparent sprite of the AREA121 sprite table. The
 *           draw-mode texture page word is 0x22C when the graph type reads 1 or
 *           2 and 0x9C otherwise; that DR_MODE is sized 0xC and appended to the
 *           primitive cursor at 0x8014598C under ordering-table index 1. The
 *           SPRT that follows is flagged semi-transparent, drawn in flat 0x80
 *           grey with CLUT 0x7B80, and takes its width, height and two texture
 *           bytes from the four-byte entry of the byte table 0x801F6800 that
 *           the entry argument selects, then is appended at 0x14 bytes. The
 *           only caller at 0x801F40DC passes entries 4 and 5, which that table
 *           fills as 0x80 by 0x18 at texture (0, 0x38) and 8 by 0x18 at
 *           texture (0, 0x50). Takes x, y and an entry index and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitSemiTransSpriteFromIndexedEntryTableWorld03Area12114_801F39F0(
    s16 x, s16 y, u8 entry) {
  SPRT* prim;
  s32 tpage;
  u32 index;

  if (GetGraphType() == 1) {
    tpage = 0x22c;
  } else if (GetGraphType() == 2) {
    tpage = 0x22c;
  } else {
    tpage = 0x9c;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  func_8014E5A0(1, 0xc);

  prim = (SPRT*)g_PrimCursor;
  SetSprt(prim);
  SetSemiTrans(prim, 1);
  setRGB0(prim, 0x80, 0x80, 0x80);

  index = (u8)entry * 4;
  prim->x0 = x;
  prim->y0 = y;
  prim->clut = 0x7b80;
  prim->w = D_801F6800[index];
  prim->h = D_801F6800[index + 1];
  prim->u0 = D_801F6800[index + 2];
  prim->v0 = D_801F6800[index + 3];
  func_8014E5A0(1, 0x14);
}
