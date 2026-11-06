#include "bof3/battle/battle03_internal.h"

/* @source 0x801E64B4
 * @behavior Second handler of the 0x801D0F20 queued-slot byte +0x01 dispatch
 * table and the fade-in mirror of the 0x801E65F8 twin: it subtracts the current
 * queued slot word +0x0C from the shared 0x8014932C halfword channel (the twin
 * adds it), opens the 0x3C0 texture page draw mode and emits the 0x0B colour
 * tile built from the slot level word +0x10 -- the level replicated into all
 * three 5-bit channels while the 0x80146374 progress byte is below 3, and only
 * its red channel otherwise. It then publishes mode 2 at 0x80149333, steps the
 * slot level up by 8 (the twin steps it down) and counts down the slot byte
 * +0x09; once that countdown expires it advances the slot state byte +0x01 and
 * re-arms the countdown at 2 (the twin clears the queued slot instead).
 * @status partial
 * @match 96.30
 * @residual Entry-block scheduling only: the build keeps the ra save at the top
 * of the entry block instead of after the texture-X constant, and hoists the
 * GetTPage 4th-argument zero into the channel load delay where the original
 * leaves a nop and fills the jal delay instead. Size, frame, calls, stores and
 * every later instruction match; 78/81 instructions, 324 bytes.
 */
void drawSlotFadeTileAndRearmCountdown(void) {
  Battle03QueuedSlot* work;
  Battle03QueuedSlot* slot;
  s32 level;

  /* This single-execution scope is the twin's spelling: it keeps the texture-X
   * constant's definition ahead of the global load in the entry block. */
  do {
    level = 0x3c0;
  } while (0);
  work = D_801EC2E0;
  D_8014932C = (u16)(D_8014932C - work->unk_0c);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, level, 0), 0);
  func_8014E5A0(1, 0x0c);
  if (BATTLE_GLOBAL_BYTE_6374 < 3u) {
    level = D_801EC2E0->unk_10;
    drawColorTilePrim(0, 0, 0x0b, (u16)((level << 10) + (level << 5) + level),
                      1);
  } else {
    drawColorTilePrim(0, 0, 0x0b, (D_801EC2E0->unk_10 << 10) & 0xfc00, 1);
  }
  slot = D_801EC2E0;
  D_80149333 = 2;
  slot->unk_10 = slot->unk_10 + 8;
  if (slot->unk_09 != 0) {
    slot->unk_09 = slot->unk_09 - 1;
  } else {
    slot->unk_01++;
    D_801EC2E0->unk_09 = 2;
  }
}
