#include "bof3/battle/battle15_internal.h"

/* @source 0x800B0118
 * @behavior Loads the work pointer at 0x1F800044 and selects a 16-byte row at
 * 0x800B6C90 using byte 0x800B6D00[byte 0x801463C9]. Stores the selected signed
 * halfword pair, sign-extended to 32 bits, at work offsets +0x0C and +0x10; rereads
 * work byte +0x08 masked with 3 for each pair index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectOffsetPairByMode(void) {
  BattleWork *work;
  u8 table_index;

  work = (BattleWork *)g_battle_work;
  table_index = D_800B6D00[D_801463C9];
  work->unk_0C = D_800B6C90[table_index].values[work->unk_08 & 3][0];
  work->unk_10 = D_800B6C90[table_index].values[work->unk_08 & 3][1];
}
