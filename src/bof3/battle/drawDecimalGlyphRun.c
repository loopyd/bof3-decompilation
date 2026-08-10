#include "bof3/battle/battle03_internal.h"

/* @behavior formats a short decimal string into the shared UI buffer, then emits a
 * run of 6x5 glyphs using palette slot `arg2` and the alternate template chosen
 * by `arg3`.
 * @source 0x801D94D4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void drawDecimalGlyphRun(s16 arg0, u16 arg1, u8 arg2, s16 arg3) {
  u8  index;
  u8  temp;
  u8  digit;
  u8*  packet;
  s16* xpos;
  u16  value;

  index = 0;
  if ((u16)arg3 == 0xffff) {
    sprintf((char*)D_80145AD4, D_801D0C70);
  } else {
    sprintf((char*)D_80145AD4, D_801D0C74, (u16)arg3);
  }
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
      /*
       * MATCHING_AID (permuter-found): the packet+8 pointer temporary keeps the
       * prologue entry-copy order s3,s5,s6,s2; without it GCC copies a2 first.
       * Remove when the allocator ordering is understood.
       */
      xpos = (s16*)(packet + 8);
      *xpos = arg0;
      *(u16*)(packet + 0xa) = arg1;
      digit = D_80145AD4[index];
      packet[0xd] = 0xd8;
      *(u16*)(packet + 0x10) = 6;
      *(u16*)(packet + 0x12) = 5;
      packet[0xc] = (digit * 6) - 0x50;
      SetSprt((SPRT*)packet);
      func_8014E5A0(1, 0x14);
    }

    arg0 += 5;
  } while (D_80145AD4[++index] != 0);
}
