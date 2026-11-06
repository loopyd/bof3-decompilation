#include "bof3/battle/battle03_internal.h"

/* @source 0x801D8450
 * @behavior Draws the battle command strip of the command selected by the low
 * byte of arg0: opens the 0x3C0-texture draw mode, emits the D_801EAE50 panel
 * tile row 5 at (x, y + 2) and the two quadrangle caps from D_801EAD30 rows
 * 0x0A and 0x0B at (x, y) and (x, y + 0x11), then outlines the rectangle inset
 * to (x + 2, y + 2) through (x + 0x25, y + 0x10) with four semi-transparent
 * sprite prims, drawing the top and left edges from texture page 1 and the
 * right and bottom edges from texture page 0 with the red/green/blue channels
 * of palette entry 1 of the CLUT bank byte D_80144952, and finally prints the
 * 8-character command label at (x + 8, y + 3) through func_8014F800. The anchor
 * comes from the signed halfword at 0x801EAEE8 and the unsigned halfword at
 * 0x801EAEEA selected by that byte, and the label is the 8-byte row of
 * 0x801EAEB0 selected by it (Atk/Abl/Use/Exa/Def/Chg/Esc). Its only in-image
 * caller, callD8450WhenIdle, passes the selection byte D_801462F4 while
 * D_80144955 is zero.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The anchor index is a named `s32` byte offset: `offset = (arg0 & 0xFF) << 3`
 * is spilled to its own frame slot and reloaded for the 0x801EAEEA read and the
 * 0x801EAEB0 label address, and each table keeps its own symbol-relative base.
 * The seed copied the statement order, operand forms and cast spelling of the
 * exact twins drawBattlePanelStrip, drawBattlePanelMessageStrip and
 * func_801D8090 and added the trailing func_8014F800 label call.
 */
void drawBattleCommandStrip(u32 arg0) {
  s32 offset;
  s16 x;
  u16 y;
  s32 palette_offset;
  u16 palette;
  s32 red;
  s32 green;
  s32 blue;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
  func_8014E5A0(1, 0x0c);
  offset = (arg0 & 0xFF) << 3;
  x = *(s16*)(D_801EAEE8 + offset);
  palette_offset = ((D_80144952 * 2) + 1) * 32;
  y = *(u16*)(D_801EAEEA + offset);
  palette = *(u16*)(PSX_PTR(u8, 0x80033A08u) + palette_offset);
  red = (palette & 0x1f) << 3;
  green = (palette >> 2) & 0xf8;
  blue = (palette >> 7) & 0xf8;
  func_801D9C80(x, (s16)(y + 2), 5, 1);
  func_801D9AB4(x, (s16)y, 0x0a, 1);
  func_801D9AB4(x, (s16)(y + 0x11), 0x0b, 1);
  drawSpritePrimTpage1((s16)(x + 2), (s16)(y + 2), (s16)(x + 0x25),
                       (s16)(y + 2), red, green, blue);
  drawSpritePrimTpage1((s16)(x + 2), (s16)(y + 3), (s16)(x + 2),
                       (s16)(y + 0x10), red, green, blue);
  drawSpritePrimTpage0((s16)(x + 0x25), (s16)(y + 2), (s16)(x + 0x25),
                       (s16)(y + 0x10), red, green, blue);
  drawSpritePrimTpage0((s16)(x + 2), (s16)(y + 0x10), (s16)(x + 0x25),
                       (s16)(y + 0x10), red, green, blue);
  func_8014F800((s16)(x + 8), (s16)(y + 3), 0, 8,
                (u32)(D_801EAEB0 + offset));
}
