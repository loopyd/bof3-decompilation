#include "bof3/battle/battle03_internal.h"

/* @source 0x801D9E9C
 * @behavior Draws the two-segment textured gauge bar at (arg0, arg1) into the
 * shared primitive cursor. The first segment is a POLY_FT4 opened by SetPolyFT4
 * with the texture page GetTPage(0, 0, 0x3c0, 0) at +0x16, the CLUT column
 * 0xe0 when arg4 is zero and 0xb0 otherwise on row 0x1e0 at +0x0e, the neutral
 * gray 0x80 in r0/g0/b0, and the corners (arg0, arg1), (arg0 + the arg2 low
 * byte, arg1), (arg0, arg1 + 7) and (arg0 + the arg2 low byte, arg1 + 7) at
 * +0x08 to +0x22, textured with the 8x8 tile spanning u 0x90..0x97 when arg4 is
 * zero and u 0x80..0x87 otherwise on texture rows 0xd8..0xdf, and appended
 * under ordering-table index 1 as a 0x28-byte primitive. The second segment is
 * the same quad continued from that x for the arg3 low byte: its corners run
 * from arg0 + the arg2 byte through arg0 + the arg2 byte + the arg3 byte on the
 * same rows, textured with the tile spanning u 0x98..0x9f and the CLUT column
 * 0xb0, and appended the same way. Its only in-image callers, the battle status
 * panel and column draws, feed it the panel anchor and the two gauge widths of
 * the drawn battler.
 * @status partial
 * @match 28.93
 * @residual Live asm diff 35/121 instructions, 476->484 bytes. The whole body,
 * both primitive sequences and every stored constant are reproduced, but the
 * original recomputes each segment's coordinates from the arguments while this
 * build shares them: the `bin/cc -da` dumps hold eight `and ...,0xff` and four
 * `addiu ...,7` after `.jump` and only two and one after `.cse`, so cse1 folds
 * the first segment's `arg0 + (arg2 & 0xff)` and `arg1 + 7` into single pseudos
 * that stay live across the four calls of the second primitive. Those two
 * pseudos take callee-saved registers, so the frame is 0x38 against the
 * original's 0x30 and the register homes shift from s0=packet/s2=arg0/
 * s1=arg1/s4=arg2/s5=arg3/s3=arg4 to packet=s2/arg0=s3/arg1=s4/arg2=s7/
 * arg3=s8/arg4=s0, first mismatch `addiu sp,sp,-56`. Twenty-eight clean-C
 * shapes were measured against that merge - `POLY_FT4*` field stores versus
 * `u8*` offsets, `(s16)`/`(u16)`/`(u8)` casts on every coordinate, s32 versus
 * u8 u/v locals, ternary, switch and explicit-else forms of the arg4 test, the
 * u/v store block before the geometry, copies of the arguments into quad-two
 * locals, and both operand orders of the sum - and every one of them leaves the
 * merge in `.cse` unchanged.
 */
void drawBattlerStatusGaugeBar(s16 arg0, s16 arg1, u16 arg2, u16 arg3,
                               u8 arg4) {
  u8* packet;
  s32 clut_x;
  u8 u_left;
  u8 u_right;

  packet = g_PrimCursor;
  SetPolyFT4((POLY_FT4*)packet);
  *(u16*)(packet + 0x16) = GetTPage(0, 0, 0x3c0, 0);
  clut_x = 0xb0;
  if (arg4 == 0) {
    clut_x = 0xe0;
  }
  *(u16*)(packet + 0xe) = GetClut(clut_x, 0x1e0);
  packet[4] = 0x80;
  packet[5] = 0x80;
  packet[6] = 0x80;
  *(s16*)(packet + 0x8) = arg0;
  *(s16*)(packet + 0xa) = arg1;
  *(s16*)(packet + 0x10) = arg0 + (arg2 & 0xff);
  *(s16*)(packet + 0x12) = arg1;
  *(s16*)(packet + 0x18) = arg0;
  *(s16*)(packet + 0x1a) = arg1 + 7;
  *(s16*)(packet + 0x20) = arg0 + (arg2 & 0xff);
  *(s16*)(packet + 0x22) = arg1 + 7;
  u_left = 0x80;
  u_right = 0x87;
  if (arg4 == 0) {
    u_left = 0x90;
    u_right = 0x97;
  }
  packet[0xc] = u_left;
  packet[0xd] = 0xd8;
  packet[0x14] = u_right;
  packet[0x15] = 0xd8;
  packet[0x1c] = u_left;
  packet[0x1d] = 0xdf;
  packet[0x24] = u_right;
  packet[0x25] = 0xdf;
  func_8014E5A0(1, 0x28);
  packet = g_PrimCursor;
  SetPolyFT4((POLY_FT4*)packet);
  *(u16*)(packet + 0x16) = GetTPage(0, 0, 0x3c0, 0);
  *(u16*)(packet + 0xe) = GetClut(0xb0, 0x1e0);
  packet[4] = 0x80;
  packet[5] = 0x80;
  packet[6] = 0x80;
  *(s16*)(packet + 0x8) = arg0 + (arg2 & 0xff);
  *(s16*)(packet + 0xa) = arg1;
  *(s16*)(packet + 0x10) = arg0 + (arg2 & 0xff) + (arg3 & 0xff);
  *(s16*)(packet + 0x12) = arg1;
  *(s16*)(packet + 0x18) = arg0 + (arg2 & 0xff);
  *(s16*)(packet + 0x1a) = arg1 + 7;
  *(s16*)(packet + 0x20) = arg0 + (arg2 & 0xff) + (arg3 & 0xff);
  *(s16*)(packet + 0x22) = arg1 + 7;
  packet[0xc] = 0x98;
  packet[0xd] = 0xd8;
  packet[0x14] = 0x9f;
  packet[0x15] = 0xd8;
  packet[0x1c] = 0x98;
  packet[0x1d] = 0xdf;
  packet[0x24] = 0x9f;
  packet[0x25] = 0xdf;
  func_8014E5A0(1, 0x28);
}
