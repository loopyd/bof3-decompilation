#include "bof3/ui/game00_internal.h"

extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* @behavior Boots the current scratch work record: submits the positional
 * effect through func_8019651C and stores the returned effect byte at 0x93 of
 * the work record published at D_80146884, sets scratch flag bit 0x20, resets
 * the three signed tint channels at 0x5D/0x5E/0x5F to the -0x40 level, sets the
 * 0x5C flag byte and the 0x04 handler index to 1, and consumes one unit of the
 * countdown halfword at 0x7E of the D_80146884 work record.
 * @source 0x801ACA14
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 0x04 store reads the work pointer through its own local ahead of the 0x7E
 * countdown update. Re-reading the scratch pointer at the store point schedules
 * that load after the countdown store, and capturing it in the same local as the
 * tint channels moves the 0x5C store base off $v0.
 */
void func_801ACA14(void) {
  struct GameWorkArea* work;
  struct GameWorkArea* tail;

  D_80146884[0x93] = func_8019651C(g_game_work, 0, 0, 0, 1);
  g_game_work->flags_00 |= 0x20;
  work = g_game_work;
  work->unk_5F = -0x40;
  work->unk_5E = -0x40;
  work->unk_5D = -0x40;
  g_game_work->flags_5C = 1;
  tail = g_game_work;
  *(u16*)(D_80146884 + 0x7e) -= 1;
  tail->field_04 = 1;
}
