#include "bof3/ui/game00_internal.h"

extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/*
 * @behavior Calls func_8019651C with the scratch work pointer and arguments 0, 0, 0, 1,
 * then stores its low return byte at D_80146884 + 0x93. Sets scratch flag bit
 * 0x20, clears bit 0x40, sets the three signed tint channels at 0x5D-0x5F to
 * -0x80, flag byte 0x5C and handler byte 0x04 to 1, and decrements the
 * D_80146884 countdown halfword at 0x7E. The helper and return byte have no
 * proven further role.
 * @source 0x801ACC08
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWorkModeWithTintMinus128(void) {
  struct GameWorkArea* work;

  D_80146884[0x93] = func_8019651C(g_game_work, 0, 0, 0, 1);
  g_game_work->flags_00 |= 0x20;
  g_game_work->flags_00 &= 0xBF;
  g_game_work->flags_5C = 1;
  work = g_game_work;
  work->unk_5F = -0x80;
  work->unk_5E = -0x80;
  work->unk_5D = -0x80;
  g_game_work->field_04 = 1;
  *(u16*)(D_80146884 + 0x7e) -= 1;
}
