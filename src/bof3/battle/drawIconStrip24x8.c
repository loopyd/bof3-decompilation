#include "bof3/battle/battle03_internal.h"

/* @behavior emits one 24x8 numeric/icon strip selected by the low byte of `arg3`
 * and palette slot `arg2`.
 * @source 0x801D9804
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawIconStrip24x8(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  s32 x;
  u8* packet;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
  func_8014E5A0(1, 0xc);

  packet = g_PrimCursor;
  *(u16*)(packet + 0xe) = GetClut((arg2 & 0xff) << 4, 0x1e0);
  packet[4] = 0x80;
  packet[5] = 0x80;
  packet[6] = 0x80;
  packet[0xc] = (u8)(((arg3 & 0xff) * 0x18) + 0x68);
  packet[0xd] = 0xd0;
  x = arg0;
  *(u16*)(packet + 0x10) = 0x18;
  *(s16*)(packet + 8) = x;
  *(s16*)(packet + 10) = (x = arg1);
  *(u16*)(packet + 0x12) = 8;
  SetSprt((SPRT*)packet);
  func_8014E5A0(1, 0x14);
}
