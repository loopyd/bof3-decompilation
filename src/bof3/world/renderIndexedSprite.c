#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* Four-byte entry table the entry argument indexes: the leading byte is the
 * sprite width, the second its height and the last two the texture u and v
 * bytes.
 * @source 0x801F52B0 @kind table
 */
extern u8 D_801F52B0[];

/* @source 0x801F3A10
 * @behavior Queues one semi-transparent sprite on the primitive cursor at
 *           0x8014598C. The draw-mode texture page word is 0x22C when the graph
 *           type reads 1 or 2 and 0x9C otherwise; that DR_MODE is appended
 *           sized 0xC under ordering-table index 1. The SPRT that follows is
 *           flagged semi-transparent, drawn in flat 0x80 grey with CLUT 0x7B80,
 *           and takes the two signed arguments as its x0 and y0 halfwords,
 *           while its width, height and two texture bytes come from the
 *           four-byte entry of the byte table 0x801F52B0 that the third
 *           argument selects, and it is appended sized 0x14. Entries 4 and 5,
 *           the pair this target's 0x801F40FC caller passes, are 0x80 by 0x18
 *           at texture (0, 0x38) and 8 by 0x18 at texture (0, 0x50). Takes x, y
 *           and an entry index and returns nothing. The 0x128 bytes differ from
 *           the span at 0x801F39F0 in emi/world03/area121/14 in the low half of
 *           its four `lbu` words alone, which carry this payload's table
 *           0x801F52B0 instead of that overlay's 0x801F6800, and the same span
 *           sits at 0x801F390C in emi/world00/area033/13, 0x801F39DC in
 *           emi/world03/area115/13 and 0x801F3948 in emi/world03/area151/13
 *           with their own tables, so this body follows that statement order
 *           with this target's table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderIndexedSprite(
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
  prim->w = D_801F52B0[index];
  prim->h = D_801F52B0[index + 1];
  prim->u0 = D_801F52B0[index + 2];
  prim->v0 = D_801F52B0[index + 3];
  func_8014E5A0(1, 0x14);
}
