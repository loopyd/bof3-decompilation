#include "bof3/bof3.h"
#include "gpu/prim.h"

/* @source 0x801EF3B4
 * @behavior Emits one 168x24 sprite panel at (0x4c, 0x30) sampled from the
 *           battle texture page GetTPage(1, 0, 0x340, 0x100) with CLUT
 *           (0, 0x1fa): stores the caller's colour byte into r0/g0/b0, zeroes
 *           u0/v0, sets width 0xa8 and height 0x18, and appends the 0x14-byte
 *           SPRT under ordering-table index 2. The panel's screen role is not
 *           established; both callers pass the neutral 0x80 shade.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitTintedPanelSprite(u8 color) {
  u8* packet;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(1, 0, 0x340, 0x100), 0);
  GpuAppendPrim(2, 0xc);

  packet = g_PrimCursor;
  *(u16*)(packet + 0xe) = GetClut(0, 0x1fa);
  *(u16*)(packet + 0x8) = 0x4c;
  *(u16*)(packet + 0xa) = 0x30;
  *(u16*)(packet + 0x10) = 0xa8;
  packet[4] = color;
  packet[5] = color;
  packet[6] = color;
  packet[0xc] = 0;
  packet[0xd] = 0;
  *(u16*)(packet + 0x12) = 0x18;
  SetSprt((SPRT*)packet);
  GpuAppendPrim(2, 0x14);
}
