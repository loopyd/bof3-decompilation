#include "bof3/ui/game00_internal.h"

/* @behavior Adds the signed delta to each non-zero signed channel byte of the
 * active work record at offsets 0x5D, 0x5E and 0x5F, clamps every updated
 * channel to at most -0x40, then reports whether the three channel bytes are
 * all -0x40 by comparing the upper three bytes of the word at 0x5C with
 * 0xC0C0C000.
 * @source 0x801C2894
 * @status exact
 * @match 100.00
 * @residual none
 * The delta is an `s8` parameter, like the sibling tint step func_801C29A0.
 * The narrow parameter is what makes the original's head: the sign extension is
 * elided because the sum is only stored back through an 8-bit `sb`, and the
 * remaining live ranges give the `$a2` delta copy plus the `$v1` channel copy
 * that fill the branch and load-delay slots.
 */
s32 func_801C2894(s8 delta) {
  if (g_game_work->unk_5D != 0) {
    g_game_work->unk_5D = g_game_work->unk_5D + delta;
    if (g_game_work->unk_5D > -0x40) {
      g_game_work->unk_5D = -0x40;
    }
  }

  if (g_game_work->unk_5E != 0) {
    g_game_work->unk_5E = g_game_work->unk_5E + delta;
    if (g_game_work->unk_5E > -0x40) {
      g_game_work->unk_5E = -0x40;
    }
  }

  if (g_game_work->unk_5F != 0) {
    g_game_work->unk_5F = g_game_work->unk_5F + delta;
    if (g_game_work->unk_5F > -0x40) {
      g_game_work->unk_5F = -0x40;
    }
  }

  return ((*(u32*)(void*)&g_game_work->flags_5C & 0xFFFFFF00u) ==
          0xC0C0C000u);
}
