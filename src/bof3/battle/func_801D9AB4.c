#include "bof3/battle/battle03_internal.h"

/* @source 0x801D9AB4
 * @behavior Draws a flat four-point quadrangle at the current primitive cursor:
 * adds the four coordinate pairs of the selected 0x10-byte-stride template row
 * at D_801EAD30 to the two corner arguments, packs the red/green/blue channels
 * of the palette entry selected by the CLUT bank byte D_80144952 and the low
 * byte of arg3 into the colour bytes, marks the primitive semi-transparent and
 * closes the 0x18-byte packet.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D9AB4(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  POLY_F4* primitive;
  s8* paletteBank;
  s32 offset;
  s32 palette_offset;

  primitive = (POLY_F4*)g_PrimCursor;
  func_8017A9A4((u32)primitive);
  offset = (arg2 & 0xff) * 16;
  primitive->x0 = arg0 + *(u16*)((u8*)D_801EAD30 + offset);
  primitive->y0 = arg1 + *(u16*)((u8*)D_801EAD30 + (offset + 2));
  primitive->x1 = arg0 + *(u16*)((u8*)D_801EAD30 + (offset + 4));
  primitive->y1 = arg1 + *(u16*)((u8*)D_801EAD30 + (offset + 6));
  primitive->x2 = arg0 + *(u16*)((u8*)D_801EAD30 + (offset + 8));
  primitive->y2 = arg1 + *(u16*)((u8*)D_801EAD30 + (offset + 10));
  primitive->x3 = arg0 + *(u16*)((u8*)D_801EAD30 + (offset + 12));
  primitive->y3 = arg1 + *(u16*)((u8*)D_801EAD30 + (offset + 14));
  paletteBank = &D_80144952;
  palette_offset = (((*paletteBank) * 2) + (arg3 & 0xff)) * 32;
  primitive->r0 = ((*(u16*)(PSX_PTR(u8, 0x80033A08u) + palette_offset) & 0x1f) * 8);
  palette_offset = (((*paletteBank) * 2) + (arg3 & 0xff)) * 32;
  primitive->g0 = (*(u16*)(PSX_PTR(u8, 0x80033A08u) + palette_offset) >> 2) & 0xf8;
  palette_offset = (((*paletteBank) * 2) + (arg3 & 0xff)) * 32;
  primitive->b0 = (*(u16*)(PSX_PTR(u8, 0x80033A08u) + palette_offset) >> 7) & 0xf8;
  SetSemiTrans(primitive, arg3 & 0xff);
  func_8014E5A0(1, 0x18);
}
