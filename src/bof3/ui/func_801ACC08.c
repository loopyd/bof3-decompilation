#include "bof3/ui/game00_internal.h"

extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/*
 * @behavior Boots the current scratch work record and steps the entry-0 work
 * mode: submits the positional effect through func_8019651C with the scratch
 * work pointer and stores the returned effect byte at 0x93 of the work record
 * published at D_80146884, sets scratch flag bit 0x20, clears flag bit 0x40,
 * sets the 0x5C flag byte and the 0x04 handler index to 1, resets the three
 * signed tint channels at 0x5D/0x5E/0x5F to the -0x80 level, and consumes one
 * unit of the countdown halfword at 0x7E of the D_80146884 work record.
 * @source 0x801ACC08
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801ACC08(void) {
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
