#include "bof3/battle/battle03_internal.h"

/* @source 0x801DA328
 * @behavior Emits one 8x8 textured battle sprite at (arg0, arg1) into the
 * shared primitive cursor: a DR_MODE carrying GetTPage(0, 0, 0x3c0, 0), then a
 * SPRT with the size words 8 and 8, the texel bytes arg2 and arg3 at +0x0C and
 * +0x0D, the CLUT halfword built from arg4 modulo 0x40 over the 0x7800 base,
 * tinted by the neutral gray 0x80 and appended under ordering-table index 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpritePrim8x8Clut(u16 arg0, u16 arg1, u8 arg2, u8 arg3, u8 arg4) {
  u8* packet;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
  func_8014E5A0(1, 0xc);

  packet = g_PrimCursor;
  *(u16*)(packet + 0x10) = 8;
  *(u16*)(packet + 0x12) = 8;
  *(u16*)(packet + 0xe) = (arg4 % 0x40) | 0x7800;
  packet[4] = 0x80;
  packet[5] = 0x80;
  packet[6] = 0x80;
  packet[0xd] = arg3;
  packet[0xc] = arg2;
  *(u16*)(packet + 0x8) = arg0;
  *(u16*)(packet + 0xa) = arg1;

  SetSprt((SPRT*)packet);
  func_8014E5A0(1, 0x14);
}
