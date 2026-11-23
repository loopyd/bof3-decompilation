#include "bof3/world/area03004_internal.h"

/* @behavior publishes the mode table byte selected by the scratch record rows at
 * +6 and +0x4A into slot +0x4B, copies that slot's 12-byte entry word into the
 * record, clears the slot and halfword fields, seeds the countdown byte with four
 * and the +0x2E halfword with 293, then advances the dispatch index.
 * @source 0x801D39B8
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializePanelMessageRun(void) {
  u8* cursor;
  u8* next;
  u8* countdown;
  u32 entry_word;
  u8* tail;
  u8 index;

  cursor = (u8*)D_1F800044;
  cursor[0x4Bu] = D_801E1F20[cursor[6u]][cursor[0x4Au]];

  next = (u8*)D_1F800044;
  index = next[0x4Bu];
  entry_word = D_801E1E78[index].word_04;
  next[0x49u] = 0u;
  countdown = (u8*)D_1F800044;
  *(s16*)(next + 0x58u) = 0;
  *(u32*)(next + 0x50u) = entry_word;
  countdown[9u] = 4u;

  tail = (u8*)D_1F800044;
  *(s16*)(tail + 0x2Eu) = 293;
  tail[1u] = (u8)(tail[1u] + 1u);
}
