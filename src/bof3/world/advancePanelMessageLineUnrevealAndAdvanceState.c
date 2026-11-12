#include "bof3/world/area03004_internal.h"

/* Shared panel origin halfword: the AREA030 panel steps submit D_80144154 + 2
 * as the second argument of the shaded-panel request func_8014F800. */
extern u16 D_80144154; /* @source 0x80144154 @kind unknown */

/* @source 0x801D3F40
 * @behavior AREA030 panel message line step (entry 11 of the state-1 handler
 * table 0x801E1F44): subtracts two from the vertical offset halfword 0x2E of
 * the work record published at the scratchpad cursor 0x1F800044 and, while that
 * offset stays below 0x1D with a nonzero revealed count in byte 0x49, copies
 * the run cursor word 0x50 into word 0x54, steps the cursor past the message
 * control codes 0x12 and 0x15, advances it one further byte while counting one
 * revealed token off that byte 0x49, and rewinds the vertical offset to 0x23
 * with the reveal timer byte 0x09 rearmed to 4. While the timer runs it counts
 * the timer down and resubmits the saved line through
 * drawMessageTokenCellQuad with the shade span (8 - (4 - timer) * 2) & 0xFE
 * plus 0x80 and the pen column 0x1D.
 * While the revealed count is nonzero it submits the shaded panel through
 * func_8014F800 at (s16)0x2E, (s16)0x80144154 + 2, 0, the revealed count and
 * the run cursor word 0x50, then, unless the entry attribute byte 0x08 of the
 * 12-byte panel entry 0x801E1E78 selected by record byte 0x4B is the 0xFF
 * sentinel, submits the sliding message through the same helper at
 * (s16)(0x2E + 0x49 * 8) with the third byte of the 3-byte row of 0x801E1F14
 * selected by that attribute. With no revealed token left it submits only that
 * sliding message, or runs func_80196070 when the attribute is the sentinel, and
 * when the attribute is not the sentinel and the vertical offset has reached
 * 0x1D it rearms the timer byte 0x09 with 4 and advances the record state index
 * byte 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */

/* The original translation unit re-reads the volatile cursor cell 0x1F800044
 * for each store group and again after a call returns, so every group keeps its
 * own record temporary instead of one shared pointer. The arm that submits the
 * revealed panel reads the shared origin halfword once per request, so it holds
 * that cell in a pointer local of its own. */
void advancePanelMessageLineUnrevealAndAdvanceState(void) {
  u8* record;
  u8* next;
  u8* stream;
  u8* step;
  u8* timer;
  u8* line;
  u8* panel;
  u8* sliding;
  u8* advance;
  u8 span;
  u8 revealed;
  u8 countdown;
  u16* origin;
  u16 offset;
  s32 entryOffset;
  s32 lineOffset;

  record = D_1F800044;
  offset = *(u16*)(record + 0x2Eu);
  *(u16*)(record + 0x2Eu) = (u16)(offset - 2u);
  if (((s16)(offset - 2u) < 0x1D) && (record[0x49u] != 0u)) {
    *(u8**)(record + 0x54u) = *(u8**)(record + 0x50u);
    stream = *(u8**)(record + 0x50u);
    if ((*stream == 0x12u) || (*stream == 0x15u)) {
      *(u8**)(record + 0x50u) = (u8*)(stream + 1);
    }

    next = D_1F800044;
    revealed = (u8)(next[0x49u] - 1u);
    *(u8**)(next + 0x50u) = (u8*)(*(u8**)(next + 0x50u) + 1);
    next[0x49u] = revealed;

    step = D_1F800044;
    *(u16*)(step + 0x2Eu) = 0x23u;
    step[9u] = 4u;
  }

  timer = D_1F800044;
  if (timer[9u] != 0u) {
    countdown = (u8)(timer[9u] - 1u);
    timer[9u] = countdown;
    panel = D_1F800044;
    drawMessageTokenCellQuad(0, *(u8**)(panel + 0x54u),
                  (u8)(((8 - (4 - panel[9u]) * 2) & 0xFEu) | 0x80u), 0x1D);
  }

  line = D_1F800044;
  if (line[0x49u] != 0u) {
    origin = &D_80144154;
    func_8014F800(*(s16*)(line + 0x2Eu), (s16)(*origin + 2u), 0,
                  line[0x49u], *(u32*)(line + 0x50u));

    sliding = D_1F800044;
    span = D_801E1E78[sliding[0x4Bu]].byte_08;
    if (span != 0xFFu) {
      entryOffset = span * 3;
      func_8014F800((s16)(*(u16*)(sliding + 0x2Eu) + sliding[0x49u] * 8),
                    (s16)(*origin + 2u), D_801E1F14[entryOffset + 2], 1u,
                    (u32)&D_801E1F14[entryOffset]);
    }
  } else {
    span = D_801E1E78[line[0x4Bu]].byte_08;
    if (span != 0xFFu) {
      lineOffset = span * 3;
      func_8014F800((s16)(*(u16*)(line + 0x2Eu) + line[0x49u] * 8),
                    (s16)(D_80144154 + 2u), D_801E1F14[lineOffset + 2], 1u,
                    (u32)&D_801E1F14[lineOffset]);

      advance = D_1F800044;
      if (*(s16*)(advance + 0x2Eu) == 0x1D) {
        advance[9u] = 4u;

        step = D_1F800044;
        step[1u] = (u8)(step[1u] + 1u);
      }
    } else {
      func_80196070();
    }
  }
}
