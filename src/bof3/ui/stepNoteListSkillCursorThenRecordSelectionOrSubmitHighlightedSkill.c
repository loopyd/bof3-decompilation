#include "bof3/ui/shop00_internal.h"

/* @source 0x801E032C
 * @behavior note-list sub-step handler reached as entry 3 of the sub-step table
 *           D_801E5CFC, which func_801DFDB8 dispatches with the UI sub-step byte
 *           D_80148652 as index: it first publishes the highlighted skill slot's
 *           ability halfword D_80148340 = the +0x12 field of the 0x14-byte
 *           ability record selected by the byte at D_801449E2 + 164 *
 *           D_801483A6 + D_801483A7, then, only while the panel-state byte
 *           D_8014839F is zero, publishes the cursor cells (D_80148624 = 1, the
 *           x halfword D_80148628 = D_801483A0 + 7 and the y halfword
 *           D_8014862A = D_801483A2 + 13 * D_801483A7 + 0x1A) and otherwise
 *           clears the visibility byte D_80148624. It then dispatches the
 *           direction func_8014E640 decodes from the pad state D_80145AA8 over
 *           the mask 0xF000: bit 0x1000 steps the ten-slot skill index
 *           D_801483A7 down and clamps it to zero, bit 0x4000 steps it up and
 *           clamps it to 9, and cue 0x100 plays when the index changed. When the
 *           pad state shares the confirm mask D_80145AC2 it records the index in
 *           D_801483A8 and advances the sub-step byte D_80148652 by one; when
 *           the pad state carries the line 0x80 it hands the highlighted slot's
 *           ability id to func_801651DC with character 0 and flag 1, clears the
 *           slot and plays cue 0x103, or plays cue 0x107 while the slot is
 *           empty. The cancel mask D_80145AC4 instead plays cues 0x102 and
 *           0x106, stores 2 into D_8014837B, 4 into D_8014839F and 1 into
 *           D_801483C3, sets D_801483A8 back to 0xFF and steps the sub-step byte
 *           down by one.
 * @status exact
 * @match 100.00
 * @residual none
 * Representation notes from the matching measurements: the two pointer locals
 * to D_801483A7 and the two record-base locals are required shapes, not
 * redundancy. The direction arms need the cell address in a register that dies
 * before the cue call, so the later reads of the same cell take their own
 * pointer (a single local assigned twice moved the first materialisation above
 * the direction call and swapped the saved-register pair, 157 of 173
 * instructions); spelling either record-base access through the other local
 * coalesced the two products and cost four instructions.
 */
void stepNoteListSkillCursorThenRecordSelectionOrSubmitHighlightedSkill(void) {
  s32 direction;
  u8  start;
  u8* cursor;
  u8* selection;
  u8* slot;
  u8* record;
  u8* slotBase;

  record = D_801449E2 + D_801483A6 * 164;
  D_80148340 = *(u16*)abilityObjects[record[D_801483A7]].control_12;

  if (D_8014839F == 0) {
    D_80148624 = 1;
    D_80148628 = D_801483A0 + 7;
    D_8014862A = D_801483A2 + (D_801483A7 * 13 + 0x1A);
  } else {
    D_80148624 = 0;
  }

  direction = func_8014E640(D_80145AA8 & 0xF000);
  cursor = &D_801483A7;
  start = *cursor;

  if (direction & 0x1000) {
    *cursor = start - 1;
    if ((s8)*cursor < 0) {
      *cursor = 0;
    }
  } else if (direction & 0x4000) {
    *cursor = start + 1;
    if ((u8)*cursor >= 10) {
      *cursor = 9;
    }
  }

  selection = &D_801483A7;
  if (start != *selection) {
    func_8015DF18(0x100);
  }

  D_801483A8 = 0xFF;
  slotBase = D_801449E2 + D_801483A6 * 164;
  slot = &slotBase[*selection];

  if (D_80145AA8 & D_80145AC2) {
    u8* step;
    func_8015DF18(0x103);
    D_801483A8 = *selection;
    step = &D_80148652;
    (*step)++;
  } else if (D_80145AA8 & 0x80) {
    if (*slot != 0) {
      func_8015DF18(0x103);
      func_801651DC(*slot, 0, 1, 0);
      *slot = 0;
    } else {
      func_8015DF18(0x107);
    }
  } else if (D_80145AA8 & D_80145AC4) {
    u8* step;
    func_8015DF18(0x102);
    func_8015DF18(0x106);
    D_8014837B = 2;
    D_8014839F = 4;
    D_801483A8 = 0xFF;
    D_801483C3 = 1;
    step = &D_80148652;
    (*step)--;
  }
}
