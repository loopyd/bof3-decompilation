#include "bof3/battle/battle03_internal.h"

/* @source 0x801D99AC
 * @behavior Draws one semi-transparent tile at the current primitive cursor:
 * publishes the tpage-0x3C0 draw mode, appends the 0xC-byte DR_MODE, then takes
 * the tile dimensions from the 2-entry row of D_801EAE50 selected by the low
 * byte of arg2 and the red/green/blue channels from scratchpad bytes 0x00
 * through 0x02 before closing the 0x10-byte packet. Both callers of this span
 * publish one value into those three scratchpad bytes and pass the row 0xB
 * (321x241, the whole screen).
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawScratchColorTile(s16 arg0, s16 arg1, s32 arg2) {
  s32 x;
  u8* packet;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 2, 0x3c0, 0), 0);
  func_8014E5A0(1, 0xc);
  packet = g_PrimCursor;
  SetTile((TILE*)packet);
  x = arg0;
  *(s16*)(packet + 8) = x;
  *(s16*)(packet + 10) = (x = arg1);
  *(u16*)(packet + 0xc) = D_801EAE50[(u8)arg2][0];
  *(u16*)(packet + 0xe) = D_801EAE50[(u8)arg2][1];
  packet[4] = SPAD_REF(u8, 0);
  packet[5] = SPAD_REF(u8, 1);
  packet[6] = SPAD_REF(u8, 2);
  SetSemiTrans((TILE*)packet, 1);
  func_8014E5A0(1, 0x10);
}
