#include "bof3/bof3.h"
#include "gpu/prim.h"

/* CLUT bank byte and palette table of the main executable, both outside this
 * EMI payload: the byte selects a 16-colour sub-palette row and the table
 * holds the 15-bit colour entries of that row. */
extern u8 D_80144952;
extern u16 D_80033A08[];

/* Shared primitive helpers of the main executable. drawQuadTemplateColor emits one
 * flat four-point quadrangle from the template row its third argument selects,
 * coloured by the CLUT bank byte and its fourth argument; func_801DA5A8 and
 * func_801DA4B4 emit one flat line between two corner pairs, coloured by the
 * three bytes given as their last arguments. None of the three is lifted at
 * its own address, so they keep their address-anchored prototypes here. */
extern void drawQuadTemplateColor(s16 arg0, s16 arg1, s32 arg2, s32 arg3); /* @source 0x801D9AB4 @kind data */
extern void func_801DA5A8(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4,
                          u8 arg5, u8 arg6);
extern void func_801DA4B4(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4,
                          u8 arg5, u8 arg6);

/* @source 0x801F039C
 * @behavior Battle-end panel frame drawer: it opens the panel draw mode for
 *   the texture page GetTPage(0, 0, 0x3C0, 0), fills the window rectangle with
 *   one tile at the primitive cursor, and then borders it with a quadrangle
 *   and four flat lines in one palette colour. The tile takes the arguments as
 *   its x origin, its width, its y origin plus two and its height, and its
 *   three colour bytes are the red, green and blue channels of the first
 *   colour entry of the 16-colour sub-palette row the shared CLUT bank byte
 *   D_80144952 selects in D_80033A08; each channel is read with its own
 *   palette access. The tile is marked semi-transparent and appended at slot
 *   1 as 0x10 bytes. The same colour is then expanded once into three bytes
 *   and reused: the quadrangle sits at the origin with template rows 8 and 9,
 *   the first pair of lines runs from (x + 2, y + 2) and (x + 2, y + 3) to the
 *   window's right and bottom edges inset by three and one, and the second
 *   pair closes the right and bottom edges, so the four lines plus the
 *   quadrangle cover the border of the filled window. The three calls that
 *   reach it come from
 *   the panel handlers func_801EFE5C, renderCountedListPanel
 *   and renderBattlerExpList, which all pass a panel
 *   origin, the window width 0x118 and a byte height. The 0x48-byte frame
 *   keeps $ra, $fp and $s0-$s7 and holds the three stacked colour arguments of
 *   the line calls at 0x10/0x14/0x18($sp); the epilogue returns no value.
 * @status partial
 * @match 93.06
 * @residual First difference at +0x88: tile field stores lack three copies.
 *   The red-channel read narrows to a byte, palette address addition operands
 *   are reversed, and the height increment is scheduled earlier.
 */
void renderPanelFrame(s32 x, s32 y, s32 width, s32 height) {
  TILE* tile;
  s32 paletteOffset;
  u16 color;
  u32 r;
  u32 g;
  u32 b;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3C0, 0), 0);
  GpuAppendPrim(1, 0xC);

  tile = (TILE*)g_PrimCursor;
  SetTile(tile);
  tile->x0 = x;
  tile->w = width;
  tile->y0 = y + 2;
  tile->h = height;

  paletteOffset = (*(s8*)&D_80144952 << 6) | 0x20;
  tile->r0 = (*(u16*)((u8*)D_80033A08 + paletteOffset) & 0x1F) << 3;
  paletteOffset = (*(s8*)&D_80144952 << 6) | 0x20;
  tile->g0 = (*(u16*)((u8*)D_80033A08 + paletteOffset) >> 2) & 0xF8;
  paletteOffset = (*(s8*)&D_80144952 << 6) | 0x20;
  tile->b0 = (*(u16*)((u8*)D_80033A08 + paletteOffset) >> 7) & 0xF8;
  SetSemiTrans(tile, 1);
  GpuAppendPrim(1, 0x10);

  paletteOffset = (*(s8*)&D_80144952 << 6) | 0x20;
  color = *(u16*)((u8*)D_80033A08 + paletteOffset);
  r = (color & 0x1F) << 3;
  g = (color >> 2) & 0xF8;
  b = (color >> 7) & 0xF8;

  drawQuadTemplateColor(x, y, 8, 1);
  drawQuadTemplateColor(x, y + height + 2, 9, 1);
  func_801DA5A8(x + 2, y + 2, x + width - 3, y + 2, r, g, b);
  height += 1;
  func_801DA5A8(x + 2, y + 3, x + 2, y + height, r, g, b);
  func_801DA4B4(x + width - 3, y + 2, x + width - 3, y + height, r, g, b);
  func_801DA4B4(x + 2, y + height, x + width - 3, y + height, r, g, b);
}
