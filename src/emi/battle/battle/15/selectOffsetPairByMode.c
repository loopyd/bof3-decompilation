#include "internal.h"

/* @source 0x800B0118 */
/* @behavior Looks up a table index, then writes the selected pair values to BattleWork offsets 0x0C and 0x10. */
void selectOffsetPairByMode(void) {
  BattleWork *work;
  u8 table_index;

  work = (BattleWork *)g_battle_work;
  table_index = D_800B6D00[D_801463C9];
  work->unk_0C = D_800B6C90[table_index].values[work->unk_08 & 3][0];
  work->unk_10 = D_800B6C90[table_index].values[work->unk_08 & 3][1];
}
