#include "bof3/world/area03004_internal.h"

/* Shared panel origin halfword: the AREA030 panel steps submit D_80144154 + 2
 * as the second argument of the shaded-panel request func_8014F800. */
extern u16 D_80144154; /* @source 0x80144154 @kind unknown */

/* @source 0x801D3C74
 * @behavior AREA030 panel message advance step (entry 10 of the state-1 handler
 * table 0x801E1F44): subtracts two from the vertical offset halfword 0x2E of the
 * work record published at the scratchpad cursor 0x1F800044. While the record
 * pause byte 0x0A is set it counts that byte down and resubmits the current
 * message run through func_801D9740 with the 3-byte row of 0x801E1F14 selected
 * by the attribute byte 0x08 of the 12-byte panel entry 0x801E1E78 indexed by
 * record byte 0x4B, the shade span 8 - 0x0A * 2 and the row argument
 * 0x125 - (4 - 0x0A) * 2. Once the pause byte is clear it counts the reveal
 * timer byte 0x09 down, and when that reaches zero it advances the token slot
 * byte 0x4A through the four-byte row of 0x801E1F20 selected by record byte 0x06,
 * wrapping the slot to zero at the third position or on the 0xFF sentinel;
 * unless the shared mode byte 0x80144125 reads 7 it then arms the shared entry
 * record 0x80143FC8 selected by the front-end pick of func_8019601c with flags
 * 1, byte 0x05 0x0F, byte 0x01 8, byte 0x06 equal to the record selector, byte
 * 0x0B zero and byte 0x4A equal to the token slot, and advances the record state
 * index byte 0x01. Finally, while the entry attribute is not the 0xFF sentinel
 * it submits the sliding message through func_8014F800 at (s16)(0x2E + 0x49 * 8),
 * (s16)0x80144154 + 2, the span row's third byte, 1 and the span row address,
 * then submits the shaded panel through the same helper at (s16)0x2E,
 * (s16)0x80144154 + 2, 0, the revealed count 0x49 and the run cursor word 0x50.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advancePanelMessageScrollAndArmEntry(void) {
  u8* record;
  u8* next;
  u8* step;
  u8* row;
  u8* advance;
  u8* panel;
  u8* cursor;
  u8 remaining;
  u32 pause;
  u8 slot;
  u8 span;
  u8 marker;
  s8 pick;
  u16 offset;
  s32 entryOffset;
  s32 panelOffset;

  /* The entry read of the pause byte keeps a wide accumulator while the local
   * re-read that feeds the span arguments stays at the record's byte width;
   * both read the same unsigned byte 0x0A and truncate only at their stores. */
  record = D_1F800044;
  offset = *(u16*)(record + 0x2Eu);
  pause = record[0x0Au];
  *(u16*)(record + 0x2Eu) = (u16)(offset - 2u);
  if (pause != 0u) {
    record[0x0Au] = (u8)(pause - 1u);
    next = D_1F800044;
    remaining = next[0x0Au];
    span = D_801E1E78[next[0x4Bu]].byte_08;
    entryOffset = span * 3;
    func_801D9740(D_801E1F14[entryOffset + 2], &D_801E1F14[entryOffset],
                  (u8)(8 - remaining * 2),
                  (s16)(0x125 - (4 - remaining) * 2));
  } else {
    record[9u] = (u8)(record[9u] - 1u);
    step = D_1F800044;
    if (step[9u] == 0u) {
      step[0x4Au] = (u8)(step[0x4Au] + 1u);
      row = D_1F800044;
      marker = row[0x4Au];
      if ((marker >= 3u) || (D_801E1F20[row[6u]][marker] == 0xFFu)) {
        row[0x4Au] = 0u;
      }
      if (modeByte != 7u) {
        pick = (s8)func_8019601c();
        if (pick != -1) {
          D_80143FC8[pick].flags_00 = 1u;
          D_80143FC8[pick].byte_05 = 15u;
          D_80143FC8[pick].unk_01 = 8u;
          D_80143FC8[pick].byte_06 = D_1F800044[6u];
          slot = D_1F800044[0x4Au];
          D_80143FC8[pick].byte_0B = 0u;
          D_80143FC8[pick].byte_4A = slot;
        }
      }
      advance = D_1F800044;
      advance[1u] = (u8)(advance[1u] + 1u);
    }
    panel = D_1F800044;
    span = D_801E1E78[panel[0x4Bu]].byte_08;
    if (span != 0xFFu) {
      panelOffset = span * 3;
      func_8014F800((s16)(*(u16*)(panel + 0x2Eu) + panel[0x49u] * 8),
                    (s16)(D_80144154 + 2u), D_801E1F14[panelOffset + 2], 1u,
                    (u32)&D_801E1F14[panelOffset]);
    }
  }

  cursor = D_1F800044;
  func_8014F800(*(s16*)(cursor + 0x2Eu), (s16)(D_80144154 + 2u), 0,
                cursor[0x49u], *(u32*)(cursor + 0x50u));
}
