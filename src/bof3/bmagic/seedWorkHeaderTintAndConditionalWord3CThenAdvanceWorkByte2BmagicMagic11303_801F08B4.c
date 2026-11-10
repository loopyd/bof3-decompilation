#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_8015DF18(u16 cue); /* @source 0x8015DF18 @kind function */
extern void func_8014D6B8(u32 selector); /* @source 0x8014D6B8 @kind function */

/* @source 0x801F08B4
 * @behavior Rewrites work words 0x38 and 0x34 and halfword 0x3E from
 * their entry values. Adds 0x01800000 to word 0x3C when byte 0xB is 2,
 * and another 0x02000000 when byte 4 is nonzero. Seeds header bytes
 * 0x25..0x28 to 0x1D, 0, 0xA0, 0; raises byte 0 bit 0x20; sets the
 * signed tint triplet at 0x5D..0x5F to -0x40 and byte 0x5C to 1.
 * Copies byte 4 to 0x2A, sets bytes 0x29 and 0x24 to 4 and 0x2B to 1,
 * clears halfword 0x2C and byte 9, then advances byte 2. Dispatches cue
 * 0x100 when byte 0xB is zero, then invokes resident helper 0x8014D6B8
 * with that byte. Takes no arguments and returns nothing; table word
 * 0x801F0BDC names this handler. Unknown field and helper roles remain
 * offset- or address-based. Pointer reloads follow stores that may alias
 * the scratchpad pointer cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHeaderTintAndConditionalWord3CThenAdvanceWorkByte2BmagicMagic11303_801F08B4(void) {
  u8 *work;
  u8 *adjust;
  u8 *header;
  u8 *clear26;
  u8 *kind;
  u8 *clear28;
  u8 *flags;
  u8 *tintR;
  u8 *tintG;
  u8 *tintB;
  u8 *mode;
  u8 *channel;
  u8 *count;
  u8 *state;
  u8 *pair;
  u8 *clear9;
  u8 *step;
  u32 saved38;
  u32 saved34;
  u16 saved3E;

  work = g_battle_work;
  saved38 = *(u32 *)(work + 0x38);
  saved34 = *(u32 *)(work + 0x34);
  saved3E = *(u16 *)(work + 0x3E);
  *(u32 *)(work + 0x34) = saved34;
  *(u32 *)(work + 0x38) = saved38;
  *(u16 *)(work + 0x3E) = saved3E;
  if (work[0xB] == 2) {
    *(u32 *)(work + 0x3C) += 0x01800000u;
  }
  adjust = g_battle_work;
  if (adjust[4] != 0) {
    *(u32 *)(adjust + 0x3C) += 0x02000000u;
  }
  header = g_battle_work;
  header[0x25] = 0x1D;
  clear26 = g_battle_work;
  clear26[0x26] = 0;
  kind = g_battle_work;
  kind[0x27] = 0xA0;
  clear28 = g_battle_work;
  clear28[0x28] = 0;
  flags = g_battle_work;
  flags[0] |= 0x20;
  tintR = g_battle_work;
  *(s8 *)(tintR + 0x5D) = -0x40;
  tintG = g_battle_work;
  *(s8 *)(tintG + 0x5E) = -0x40;
  tintB = g_battle_work;
  *(s8 *)(tintB + 0x5F) = -0x40;
  mode = g_battle_work;
  mode[0x5C] = 1;
  channel = g_battle_work;
  channel[0x2A] = channel[4];
  count = g_battle_work;
  count[0x29] = 4;
  state = g_battle_work;
  state[0x24] = 4;
  pair = g_battle_work;
  pair[0x2B] = 1;
  clear9 = g_battle_work;
  *(u16 *)(pair + 0x2C) = 0;
  clear9[9] = 0;
  step = g_battle_work;
  step[2]++;
  if (g_battle_work[0xB] == 0) {
    func_8015DF18(0x100);
  }
  func_8014D6B8(g_battle_work[0xB]);
}
