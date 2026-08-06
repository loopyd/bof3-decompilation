#include "internal.h"

/* @behavior appends one triple `(byte, byte, word)` into the 16-entry UI ring and
 * advances the ring tail.
 * @source 0x801DE8C0
 */
void pushUiRingTriple(s8 arg0, s8 arg1, u32 arg2) {
  u8 index;

  uiRingEntries[uiRingTail].unk_00 = (u8)arg0;
  uiRingEntries[uiRingTail].unk_01 = (u8)arg1;
  index = uiRingTail;
  uiRingEntries[index].unk_04 = arg2;
  uiRingTail = (index + 1u) & 0x0fu;
}
