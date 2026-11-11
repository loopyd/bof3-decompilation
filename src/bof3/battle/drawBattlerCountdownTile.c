#include "bof3/battle/battle03_internal.h"

/* Palette row of the battler countdown tiles: one byte per battler index,
 * read as the low six bits of the tile CLUT halfword. */
typedef struct {
  u8 entries[7];
} Battle03CountdownPaletteRows;

/* @source 0x801D0C64 @kind table */
extern Battle03CountdownPaletteRows D_801D0C64;

/* @source 0x801D7D10
 * @behavior Draws the countdown tile of battler arg0 at (arg1, arg2) stretched
 * to arg3 by arg4 pixels: opens the draw mode of the GetTPage(0, 0, 0x100,
 * 0x100) texture page and emits one POLY_FT4 through SetPolyFT4 with the
 * corners (arg1, arg2), (arg1 + arg3, arg2), (arg1, arg2 + arg4) and
 * (arg1 + arg3, arg2 + arg4), the tint byte arg5 in r0/g0/b0, the 24-pixel
 * tile of battler arg0 from texel (arg0 * 0x18, 0xE0) to (arg0 * 0x18 + 0x18,
 * 0xF8) and the texture page at +0x16, then the CLUT at +0x0E built from
 * palette entry arg0 of the overlay table D_801D0C64 masked to six bits over
 * the 0x7800 base, and appends it under ordering-table index 1 as a 0x28-byte
 * primitive.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawBattlerCountdownTile(u8 arg0, u16 arg1, u16 arg2, u16 arg3, u8 arg4,
                              u8 arg5) {
  Battle03CountdownPaletteRows rows;
  u8* packet;
  u8 size;

  rows = D_801D0C64;
  size = arg3;
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x100, 0x100), 0);
  func_8014E5A0(1, 0xc);

  packet = g_PrimCursor;
  SetPolyFT4((POLY_FT4*)packet);
  packet[0x4] = arg5;
  packet[0x5] = arg5;
  packet[0x6] = arg5;
  *(u16*)(packet + 0x8) = arg1;
  *(u16*)(packet + 0xa) = arg2;
  *(u16*)(packet + 0x10) = arg1 + size;
  *(u16*)(packet + 0x12) = arg2;
  *(u16*)(packet + 0x18) = arg1;
  *(u16*)(packet + 0x1a) = arg2 + arg4;
  *(u16*)(packet + 0x20) = arg1 + size;
  *(u16*)(packet + 0x22) = arg2 + arg4;
  packet[0xc] = arg0 * 0x18;
  packet[0xd] = 0xe0;
  packet[0x14] = arg0 * 0x18 + 0x18;
  packet[0x15] = 0xe0;
  packet[0x1c] = arg0 * 0x18;
  packet[0x1d] = 0xf8;
  packet[0x24] = arg0 * 0x18 + 0x18;
  packet[0x25] = 0xf8;
  *(u16*)(packet + 0x16) = GetTPage(0, 0, 0x100, 0x100);
  *(u16*)(packet + 0xe) = (rows.entries[arg0] & 0x3f) | 0x7800;
  func_8014E5A0(1, 0x28);
}
