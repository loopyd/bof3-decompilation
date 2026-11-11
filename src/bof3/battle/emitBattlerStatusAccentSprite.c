#include "bof3/battle/battle03_internal.h"

/* @source 0x801DA078
 * @behavior Emits one 16x8 textured battle sprite at (arg0, arg1) into the
 * shared primitive cursor: a DR_MODE carrying GetTPage(0, 1, 0x3c0, 0), then a
 * SPRT with the size words 0x10 and 8, the CLUT 0x7800, texture row 0xd8 and
 * texture column arg2 * 16, tinted by the neutral gray 0x80 and appended under
 * ordering-table index 1. drawBattlerStatusColumns draws the two column accents
 * of each battler status column with it, and the still-assembler func_801D8DF8
 * draws the column-0 accent at (x + 2, y + 0xA).
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitBattlerStatusAccentSprite(u16 arg0, u16 arg1, u8 arg2) {
  u8* packet;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x3c0, 0), 0);
  func_8014E5A0(1, 0xc);

  packet = g_PrimCursor;
  *(u16*)(packet + 0x10) = 0x10;
  *(u16*)(packet + 0x12) = 8;
  *(u16*)(packet + 0xe) = 0x7800;
  packet[4] = 0x80;
  packet[5] = 0x80;
  packet[6] = 0x80;
  packet[0xd] = 0xd8;
  packet[0xc] = arg2 * 16;
  *(u16*)(packet + 0x8) = arg0;
  *(u16*)(packet + 0xa) = arg1;

  SetSprt((SPRT*)packet);
  func_8014E5A0(1, 0x14);
}
