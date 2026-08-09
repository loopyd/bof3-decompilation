#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019A848
 * @behavior dispatches the work-area handler selected by field 0x06, then
 * applies signed coordinate deltas to the halfwords at offsets 0x2E/0x30.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8019A848(void)
{
  struct GameWorkArea* work;

  D_801C80C8[g_game_work->unk_06]();
  work = g_game_work;
  *(u16*)((u8*)work + 0x2E) += *(s32*)((u8*)work + 0x0C);
  *(u16*)((u8*)work + 0x30) -= *(s32*)((u8*)work + 0x10);
}
