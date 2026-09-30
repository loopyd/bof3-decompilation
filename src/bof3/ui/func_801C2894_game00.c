#include "bof3/ui/game00_internal.h"

/* @behavior Adds the signed delta to each non-zero signed channel byte of the
 * active work record at offsets 0x5D, 0x5E and 0x5F, clamps every updated
 * channel to at most -0x40, then reports whether the three channel bytes are
 * all -0x40 by comparing the upper three bytes of the word at 0x5C with
 * 0xC0C0C0.
 * @source 0x801C2894
 * @status partial
 * @match 79.10
 * @residual first mismatch +0x0008: the original copies the incoming $a0 into
 * $a2 before the first test and then reuses the freed $a0 for the folded
 * g_game_work reloads of the 0x5E/0x5F blocks, while this clean-C shape keeps
 * the delta in $a0 and reloads those two pointers into $a1; the head therefore
 * also fills the first load-delay slot with a nop instead of the frame adjust.
 * Instruction count (67), size (268 bytes) and frame (24 bytes) are already
 * exact and only the head and the two reload registers differ (53/67
 * instructions). A conditional delta local (assignment inside the 0x5D block)
 * reproduced the entry copy but scheduled it into the clamp branch delay slot
 * and added one instruction (59/68, 272 bytes), so that shape was reverted
 * under the allocation-ladder structural rule.
 */
s32 func_801C2894(s32 arg0) {
  if (g_game_work->unk_5D != 0) {
    g_game_work->unk_5D = g_game_work->unk_5D + arg0;
    if (g_game_work->unk_5D > -0x40) {
      g_game_work->unk_5D = -0x40;
    }
  }

  if (g_game_work->unk_5E != 0) {
    g_game_work->unk_5E = g_game_work->unk_5E + arg0;
    if (g_game_work->unk_5E > -0x40) {
      g_game_work->unk_5E = -0x40;
    }
  }

  if (g_game_work->unk_5F != 0) {
    g_game_work->unk_5F = g_game_work->unk_5F + arg0;
    if (g_game_work->unk_5F > -0x40) {
      g_game_work->unk_5F = -0x40;
    }
  }

  return ((*(u32*)(void*)&g_game_work->flags_5C & 0xFFFFFF00u) ==
          0xC0C0C000u);
}
