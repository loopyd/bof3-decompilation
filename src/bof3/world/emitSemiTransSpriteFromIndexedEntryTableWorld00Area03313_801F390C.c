#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* Four-byte records: width, height, texture u and texture v.
 * @source 0x801F4FA0 @kind table
 */
extern u8 D_801F4FA0[];

/* @source 0x801F390C
 * @behavior Emits one semi-transparent flat-grey sprite at the signed x and y
 *           arguments, using the width, height and texture coordinates from the
 *           four-byte record at 0x801F4FA0 selected by the byte entry argument.
 *           Appends a 0xC-byte draw mode at ordering-table index 1 with texture
 *           page 0x22C for graph types 1 or 2, otherwise 0x9C, then appends the
 *           0x14-byte sprite with colour 0x80 and CLUT 0x7B80. Returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitSemiTransSpriteFromIndexedEntryTableWorld00Area03313_801F390C(
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
  prim->w = D_801F4FA0[index];
  prim->h = D_801F4FA0[index + 1];
  prim->u0 = D_801F4FA0[index + 2];
  prim->v0 = D_801F4FA0[index + 3];
  func_8014E5A0(1, 0x14);
}
