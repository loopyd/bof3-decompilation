#include "internal.h"

/* @behavior appends one triple `(byte, byte, word)` into the 16-entry UI ring and
 * advances the ring tail.
 * @source 0x801DE8C0
 */
void battle03_ui_ring_push_triple(s8 arg0, s8 arg1, u32 arg2) {
  u8 index;

  battle03UiRingEntries[battle03UiRingTail].unk_00 = (u8)arg0;
  battle03UiRingEntries[battle03UiRingTail].unk_01 = (u8)arg1;
  index = battle03UiRingTail;
  battle03UiRingEntries[index].unk_04 = arg2;
  battle03UiRingTail = (index + 1u) & 0x0fu;
}
