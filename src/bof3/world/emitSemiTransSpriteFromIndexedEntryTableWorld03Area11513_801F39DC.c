#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* Four-byte sprite records in this target's tail blob: the byte at 0x00 is the
 * sprite width, 0x01 the height, 0x02 the texture u coordinate and 0x03 the
 * texture v coordinate. The third argument of the emitter below selects one
 * record, which the local marker drawer walks from 0 to 3 plus their incremented
 * variants.
 * @source 0x801F4F84 @kind table
 */
extern u8 D_801F4F84[];

/* @source 0x801F39DC
 * @behavior Emits one semi-transparent flat-grey sprite. The draw-mode texture
 *           page word is 0x22C when the graph type reads 1 or 2 and 0x9C
 *           otherwise; that DR_MODE is opened on the primitive cursor with a
 *           zero tail word, sized 0xC and appended through the shared primitive
 *           append helper under ordering-table index 1. The SPRT that follows is
 *           flagged semi-transparent, drawn in the flat colour 0x80 with CLUT
 *           0x7B80, placed at the two signed arguments, and takes its width,
 *           height and two texture bytes from the four-byte record of the byte
 *           table 0x801F4F84 that the third argument selects, then is appended
 *           at 0x14 bytes. Takes x, y and a record index, returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitSemiTransSpriteFromIndexedEntryTableWorld03Area11513_801F39DC(
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
  prim->w = D_801F4F84[index];
  prim->h = D_801F4F84[index + 1];
  prim->u0 = D_801F4F84[index + 2];
  prim->v0 = D_801F4F84[index + 3];
  func_8014E5A0(1, 0x14);
}
