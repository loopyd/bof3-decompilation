#include "bof3/world/area01613_internal.h"

/* @behavior emits one semi-transparent sprite using the selected sprite record.
 * @source 0x801F39D8
 * @status exact
 */
void emitSemiTransparentSprite(s16 arg0, s16 arg1, u8 arg2) {
  SPRT* primitive;
  s32   tpage;
  u32   index;

  if (GetGraphType() == 1) {
    tpage = 0x22c;
  } else if (GetGraphType() == 2) {
    tpage = 0x22c;
  } else {
    tpage = 0x9c;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  func_8014E5A0(1, 0x0c);

  primitive = (SPRT*)g_PrimCursor;
  SetSprt(primitive);
  SetSemiTrans(primitive, 1);
  setRGB0(primitive, 0x80, 0x80, 0x80);

  index = (arg2 & 0xff) << 2;
  primitive->x0 = arg0;
  primitive->y0 = arg1;
  primitive->clut = 0x7b80;
  primitive->w = D_801F513C[index];
  primitive->h = D_801F513C[index + 1];
  primitive->u0 = D_801F513C[index + 2];
  primitive->v0 = D_801F513C[index + 3];

  func_8014E5A0(1, 0x14);
}
