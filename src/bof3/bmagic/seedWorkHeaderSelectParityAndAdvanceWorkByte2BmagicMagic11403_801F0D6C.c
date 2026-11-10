#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 @kind function */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801F0D6C
 * @behavior Seeds work bytes 0x25, 0x26, 0x27 and 0x28 with 0x1D, 0,
 * 0x1A and 1. Sets byte 0x2A to (byte 8 - 1) & 1, byte 0x29 to 4,
 * and clears byte 0x24. One work-pointer load feeds the clear of byte
 * 0x2B and halfword 0x2C; a separate load feeds the clear of byte 9.
 * Reloads the pointer to advance byte 2, calls resident helper 0x8014D6B8
 * with zero, then calls resident helper 0x8014D290. Takes no arguments
 * and returns nothing. The two-entry handler table at 0x801F123C names
 * this function as entry zero. Unknown field and helper roles remain
 * offset- or address-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHeaderSelectParityAndAdvanceWorkByte2BmagicMagic11403_801F0D6C(void) {
  u8 *header;
  u8 *clear26;
  u8 *kind;
  u8 *mode;
  u8 *parity;
  u8 *count;
  u8 *clear24;
  u8 *pair;
  u8 *clear9;
  u8 *step;

  header = g_battle_work;
  header[0x25] = 0x1D;
  clear26 = g_battle_work;
  clear26[0x26] = 0;
  kind = g_battle_work;
  kind[0x27] = 0x1A;
  mode = g_battle_work;
  mode[0x28] = 1;
  parity = g_battle_work;
  parity[0x2A] = (parity[8] - 1) & 1;
  count = g_battle_work;
  count[0x29] = 4;
  clear24 = g_battle_work;
  clear24[0x24] = 0;
  pair = g_battle_work;
  pair[0x2B] = 0;
  clear9 = g_battle_work;
  *(u16 *)(pair + 0x2C) = 0;
  clear9[9] = 0;
  step = g_battle_work;
  step[2]++;
  func_8014D6B8(0);
  func_8014D290();
}
