#include "bof3/battle/battle03_internal.h"

/* @source 0x801DA14C
 * @behavior Emits one 4x4 textured battle quad at (arg0, arg1): a POLY_FT4
 * opened by SetPolyFT4 with the texture page GetTPage(0, 0, 0x3c0, 0) at
 * +0x16, the CLUT of column (D_80144952 * 2 + arg3) * 0x10 on row 0x1e1 at
 * +0x0e, the neutral gray 0x80 in r0/g0/b0 and the corners (arg0, arg1),
 * (arg0 + 4, arg1), (arg0, arg1 + 4) and (arg0 + 4, arg1 + 4) at +0x08 to
 * +0x22. Its texture window comes from the 4-byte row {u, v, du, dv} of the
 * overlay's own table D_801EAF2C selected by the low byte of arg2: u0/u2 take
 * u, u1/u3 add du, v0/v1 take v less 8 rows per arg3 band, and v2/v3 add dv to
 * that same base. The four rows at 0x801EAF2C are {0x34, 0xf4, 4, 4},
 * {0x37, 0xf4, 0xfc, 4}, {0x34, 0xf7, 4, 0xfc} and {0x37, 0xf7, 0xfc, 0xfc},
 * so du and dv are signed texel offsets stored as bytes and added modulo 0x100.
 * The quad is marked semi-transparent with the arg3 byte and appended under
 * ordering-table index 1 as a 0x28-byte primitive.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpritePrimFt4Clut(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  u8* packet;
  s32 band;

  packet = g_PrimCursor;
  SetPolyFT4((POLY_FT4*)packet);
  *(u16*)(packet + 0x16) = GetTPage(0, 0, 0x3c0, 0);
  band = arg3 & 0xff;
  *(u16*)(packet + 0xe) = GetClut(((D_80144952 * 2) + band) * 0x10, 0x1e1);
  packet[4] = 0x80;
  packet[5] = 0x80;
  packet[6] = 0x80;
  *(s16*)(packet + 0x8) = arg0;
  *(s16*)(packet + 0xa) = arg1;
  *(s16*)(packet + 0x10) = arg0 + 4;
  *(s16*)(packet + 0x12) = arg1;
  *(s16*)(packet + 0x18) = arg0;
  *(s16*)(packet + 0x1a) = arg1 + 4;
  *(s16*)(packet + 0x20) = arg0 + 4;
  *(s16*)(packet + 0x22) = arg1 + 4;
  packet[0xc] = D_801EAF2C[(u8)arg2][0];
  packet[0xd] = D_801EAF2C[(u8)arg2][1] - band * 8;
  packet[0x14] = D_801EAF2C[(u8)arg2][0] + D_801EAF2C[(u8)arg2][2];
  packet[0x15] = D_801EAF2C[(u8)arg2][1] - band * 8;
  packet[0x1c] = D_801EAF2C[(u8)arg2][0];
  packet[0x1d] = D_801EAF2C[(u8)arg2][3] + (D_801EAF2C[(u8)arg2][1] - band * 8);
  packet[0x24] = D_801EAF2C[(u8)arg2][0] + D_801EAF2C[(u8)arg2][2];
  packet[0x25] = D_801EAF2C[(u8)arg2][3] + (D_801EAF2C[(u8)arg2][1] - band * 8);
  SetSemiTrans((POLY_FT4*)packet, band);
  func_8014E5A0(1, 0x28);
}
