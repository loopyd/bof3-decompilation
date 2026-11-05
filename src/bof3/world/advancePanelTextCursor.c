#include "bof3/world/area03004_internal.h"

/* Shared panel origin halfword: the AREA030 panel steps submit D_80144154 + 2
 * as the second argument of the shaded-panel request func_8014F800. */
extern u16 D_80144154; /* @source 0x80144154 @kind unknown */

/* @source 0x801D3A54
 * @behavior AREA030 panel message step (entry 9 of the state-1 handler table
 * 0x801E1F44): counts the reveal timer byte 0x09 and the vertical offset
 * halfword 0x2E of the work record published at the scratchpad cursor
 * 0x1F800044 down by one and by two. While the timer runs it submits the
 * partially revealed message run at record word 0x50 through func_801D9740 with
 * the shade span (8 - 0x09 * 2) & 0xFE and the row argument
 * 0x09 * 2 + 0x11D. Once the timer reaches zero it steps the run cursor past
 * the message control codes 0x12 and 0x15 (one payload byte each, as in
 * countMessageTokens), counts one revealed token in byte 0x49, rearms the timer
 * with 4 and, once that count reaches the limit byte 0x00 of the 12-byte panel
 * entry table 0x801E1E78, clamps the count to the limit, loads the pause byte
 * 0x0A (4, or 0 when the entry attribute byte 0x08 is the 0xFF sentinel),
 * reloads the timer from the entry countdown byte 0x09, advances the record
 * state index byte 0x01 and repoints the run cursor at the entry word 0x04.
 * Finally, while the record has revealed tokens, it submits the shaded panel
 * through func_8014F800 at (s16)record 0x2E, (s16)D_80144154 + 2, the revealed
 * count and the entry word 0x04.
 * @status exact
 * @match 100.00
 * @residual none
 */

/* The original translation unit re-reads the volatile cursor cell 0x1F800044
 * once per store group and each of those reads takes its own register, so every
 * group keeps its own record temporary instead of one shared pointer. */
void advancePanelTextCursor(void) {
  u8* record;
  u8* next;
  u8* panel;
  u8* step;
  u8* rearm;
  u8* entry;
  u8* stream;
  u8* repoint;
  u8 index;
  u8 count;
  u8 limit;
  u16 offset;
  u8 revealed;

  record = D_1F800044;
  record[9] = (u8)(record[9] - 1u);
  offset = *(u16*)(record + 0x2Eu);
  next = D_1F800044;
  *(u16*)(record + 0x2Eu) = (u16)(offset - 2u);
  count = next[9];
  if (count == 0u) {
    stream = *(u8**)(next + 0x50u);
    if ((*stream == 0x12u) || (*stream == 0x15u)) {
      *(u8**)(next + 0x50u) = (u8*)(stream + 1);
    }

    step = D_1F800044;
    revealed = (u8)(step[0x49u] + 1u);
    *(u8**)(step + 0x50u) = (u8*)(*(u8**)(step + 0x50u) + 1);
    step[0x49u] = revealed;

    rearm = D_1F800044;
    rearm[9] = 4u;

    entry = D_1F800044;
    index = entry[0x4Bu];
    limit = D_801E1E78[index].byte_00;
    if ((u32)entry[0x49u] >= (u32)limit) {
      entry[0x49u] = limit;

      entry = D_1F800044;
      index = entry[0x4Bu];
      if (D_801E1E78[index].byte_08 != 0xFFu) {
        entry[0x0Au] = 4u;
      } else {
        entry[0x0Au] = 0u;
      }

      entry = D_1F800044;
      index = entry[0x4Bu];
      entry[9] = D_801E1E78[index].byte_09;

      entry = D_1F800044;
      index = entry[0x4Bu];
      repoint = (u8*)D_801E1E78[index].word_04;
      entry[1u] = (u8)(entry[1u] + 1u);
      *(u8**)(entry + 0x50u) = repoint;
    }
  } else {
    func_801D9740(0, *(u8**)(next + 0x50u), (u8)((8 - count * 2) & 0xFE),
                  (s16)(count * 2 + 0x11D));
  }

  panel = D_1F800044;
  if (panel[0x49u] != 0u) {
    func_8014F800(*(s16*)(panel + 0x2Eu), (s16)(D_80144154 + 2u), 0,
                  panel[0x49u], D_801E1E78[panel[0x4Bu]].word_04);
  }
}
