#include "bof3/battle/battle03_internal.h"

/* @source 0x801D9AB4
 * @behavior draws a four-point primitive from a coordinate template and palette.
 * @status partial
 * @match 84.35
 * @residual stack-frame size and palette-load width differ
 */
void func_801D9AB4(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  POLY_F4* primitive;
  s8* paletteBank;
  u16* coordinateEntry;
  s32 index;

  primitive = (POLY_F4*)g_PrimCursor;
  func_8017A9A4((u32)primitive);
  index = (arg2 & 0xff) * 8;
  coordinateEntry = &D_801EAD30[index];
  primitive->x0 = arg0 + *coordinateEntry;
  coordinateEntry = &D_801EAD30[index + 1];
  primitive->y0 = arg1 + *coordinateEntry;
  coordinateEntry = &D_801EAD30[index + 2];
  primitive->x1 = arg0 + *coordinateEntry;
  coordinateEntry = &D_801EAD30[index + 3];
  primitive->y1 = arg1 + *coordinateEntry;
  coordinateEntry = &D_801EAD30[index + 4];
  primitive->x2 = arg0 + *coordinateEntry;
  coordinateEntry = &D_801EAD30[index + 5];
  primitive->y2 = arg1 + *coordinateEntry;
  coordinateEntry = &D_801EAD30[index + 6];
  primitive->x3 = arg0 + *coordinateEntry;
  coordinateEntry = &D_801EAD30[index + 7];
  primitive->y3 = arg1 + *coordinateEntry;
  paletteBank = &D_80144952;
  primitive->r0 = (D_80033A08[(((*paletteBank) * 2) + (arg3 & 0xff)) * 16] & 0x1f) << 3;
  primitive->g0 = (D_80033A08[(((*paletteBank) * 2) + (arg3 & 0xff)) * 16] >> 2) & 0xf8;
  primitive->b0 = (D_80033A08[(((*paletteBank) * 2) + (arg3 & 0xff)) * 16] >> 7) & 0xf8;
  SetSemiTrans(primitive, arg3 & 0xff);
  func_8014E5A0(1, 0x18);
}
