#include "bof3/ui/game00_internal.h"

/* @behavior Adds the signed delta to each of the three signed tint channel
 * bytes at offsets 0x5D, 0x5E and 0x5F of the active work record published at
 * the scratchpad cursor, leaves a channel already at the -0x80 floor untouched,
 * clamps each updated channel to at least -0x80, then reports whether all three
 * channel bytes are -0x80 by comparing the upper three bytes of the word at
 * 0x5C with 0x80808000.
 * @source 0x801C29A0
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801C29A0(s8 delta) {
  s16 value;

  if (g_game_work->unk_5D != -0x80) {
    value = g_game_work->unk_5D - delta;
    if (value < -0x80) {
      g_game_work->unk_5D = -0x80;
    } else {
      g_game_work->unk_5D = value;
    }
  }

  if (g_game_work->unk_5E != -0x80) {
    value = g_game_work->unk_5E - delta;
    if (value < -0x80) {
      g_game_work->unk_5E = -0x80;
    } else {
      g_game_work->unk_5E = value;
    }
  }

  if (g_game_work->unk_5F != -0x80) {
    value = g_game_work->unk_5F - delta;
    if (value < -0x80) {
      g_game_work->unk_5F = -0x80;
    } else {
      g_game_work->unk_5F = value;
    }
  }

  return ((*(u32*)(void*)&g_game_work->flags_5C & 0xFFFFFF00u) ==
          0x80808000u);
}
