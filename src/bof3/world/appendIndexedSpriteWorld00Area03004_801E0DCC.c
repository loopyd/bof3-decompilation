#include "bof3/world/area03004_internal.h"

/* @source 0x801E0DCC
 * @behavior Build a gray sprite at the primitive cursor. The low byte of spriteIndex
 * selects a 16-byte record supplying CLUT coordinates, dimensions and UV
 * coordinates. Store x and y, append 20 bytes under the low byte of drawMode,
 * and return the sprite address.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* appendIndexedSpriteWorld00Area03004(s32 spriteIndex, s32 drawMode, s16 x, s16 y) {
  SPRT* primitive;

  primitive = (SPRT*)g_PrimCursor;
  SetSprt(primitive);
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  primitive->x0 = x;
  primitive->y0 = y;
  primitive->clut = (D_801E2424[(u8)spriteIndex].clut_y << 6) |
                    ((s32)D_801E2424[(u8)spriteIndex].clut_x >> 4 & 0x3f);
  primitive->w = D_801E2424[(u8)spriteIndex].width;
  primitive->h = D_801E2424[(u8)spriteIndex].height;
  primitive->u0 = D_801E2424[(u8)spriteIndex].u;
  primitive->v0 = D_801E2424[(u8)spriteIndex].v;
  func_8014E5A0((u8)drawMode, 0x14);
  return (u8*)primitive;
}
