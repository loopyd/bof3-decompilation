#include "bof3/battle/battle03_internal.h"

/* @source 0x801D9684
 * @behavior Formats arg3 through the 3-column decimal format string at
 * 0x801D0C78 into the shared UI buffer, then emits a run of 8x8 glyphs: for
 * every buffer byte other than the 0x20 pad it converts the digit in place,
 * takes the CLUT of palette row 0x1e0 column (arg2 & 0xff) and appends one
 * semi-transparent SPRT at (arg0, arg1) tinted by the neutral gray 0x80, with
 * texture row 0xd0 and texel column digit * 8 - 0x50, advancing arg0 by 8
 * until the next buffer byte is 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawDecimalGlyphRun8x8(s32 arg0, s32 arg1, s32 arg2, u16 arg3) {
  u8   index;
  u8   temp;
  u8   digit;
  u8*  packet;
  s16* xpos;
  u16  value;

  index = 0;
  sprintf((char*)D_80145AD4, D_801D0C78, (u16)arg3);

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x3c0, 0), 0);
  func_8014E5A0(1, 0xc);

  do {
    temp = D_80145AD4[index];
    if (temp != ' ') {
      D_80145AD4[index] = temp - 0x30;
      packet = g_PrimCursor;
      value = GetClut((arg2 & 0xff) << 4, 0x1e0);
      *(u16*)(packet + 0xe) = value;
      packet[4] = 0x80;
      packet[5] = 0x80;
      packet[6] = 0x80;
      xpos = (s16*)(packet + 8);
      *xpos = arg0;
      *(u16*)(packet + 0xa) = arg1;
      digit = D_80145AD4[index];
      packet[0xd] = 0xd0;
      *(u16*)(packet + 0x10) = 8;
      *(u16*)(packet + 0x12) = 8;
      packet[0xc] = (digit * 8) - 0x50;
      SetSprt((SPRT*)packet);
      func_8014E5A0(1, 0x14);
    }

    arg0 += 8;
  } while (D_80145AD4[++index] != 0);
}
