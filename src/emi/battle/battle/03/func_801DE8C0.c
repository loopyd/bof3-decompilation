#include "internal.h"

/* @behavior appends one triple `(byte, byte, word)` into the 16-entry UI ring and
 * advances the ring tail.
 * @source 0x801DE8C0
 */
void func_801DE8C0(s8 arg0, s8 arg1, u32 arg2) {
  u8 index;

  BATTLE_UI_RING_BYTE0(BATTLE_UI_RING_HEAD) = (u8)arg0;
  BATTLE_UI_RING_BYTE1(BATTLE_UI_RING_HEAD) = (u8)arg1;
  index = BATTLE_UI_RING_HEAD;
  BATTLE_UI_RING_WORD2(index) = arg2;
  BATTLE_UI_RING_HEAD = (index + 1u) & 0x0fu;
}
