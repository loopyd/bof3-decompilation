#include "bof3/battle/battle03_internal.h"

/* @source 0x801E2170
 * @behavior Scans the eight enemy work records; for every occupied record it
 * publishes the record pointer as the current enemy work (scratchpad +0x44 and
 * the 0x801EB4E8 global), then runs the record's mode handler table entry: the
 * 0x801EB298 table when the battle flag at 0x801462EA is set -- after calling
 * the record's optional +0xE4 handler with argument 2 -- and otherwise the
 * 0x801EB294 table, both indexed by the record's +0xF0 byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E2170(void) {
  u8                 index;
  Battle03EnemyWork* work;

  for (index = 0; index < 8; index++) {
    if (D_801EB630[index].unk_00 == 0) {
      continue;
    }
    work = &D_801EB630[index];
    BATTLE_ENEMY_SCRATCH_PTR = work;
    battleCurrentEnemyWorkState = work;
    if (BATTLE_GLOBAL_BYTE_62EA != 0) {
      if (work->unk_01 != 0) {
        work->unk_e4(2);
      }
      BATTLE_ENEMY_DISPATCH_TABLE_B[battleCurrentEnemyWorkState->unk_f0]();
    } else {
      BATTLE_ENEMY_DISPATCH_TABLE_A[work->unk_f0]();
    }
  }
}
