#include "bof3/battle/battle03_internal.h"

/* @source 0x801D9AB4
 * @behavior Draws a flat four-point quadrangle at the current primitive cursor:
 * adds the selected 0x10-byte-stride coordinate template entry at D_801EAD30 to
 * the two corner arguments for the four vertices, packs the red/green/blue
 * channels of the palette entry selected by the CLUT bank byte D_80144952 and
 * the low byte of arg3 into the colour bytes, marks the primitive
 * semi-transparent and closes the 0x18-byte packet.
 * @status partial
 * @match 84.35
 * @residual live asm diff 97/115 instructions, 460 versus 460 bytes. First
 * mismatch +0x00 is the frame: the original reserves 40 bytes (six saved
 * registers plus the 16-byte argument area) while this build reserves 64
 * because the canonical compiler keeps 24 bytes of unused local space for the
 * coordinate-template address temporaries; the direct-index and single-pointer
 * shapes were measured (112/56 bytes of locals, worse code), so the template
 * access form above is the closest clean shape. Two further residuals sit
 * behind it: the three D_80033A08 palette reads expand as `addu $at,$at,$v0`
 * here and as `addu $at,$v0,$at` in the original, and the first palette read
 * narrows to lbu instead of the original lhu. A flag-search over the 52
 * reviewed profiles kept the canonical -O2 profile as the best score (no exact
 * match).
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
