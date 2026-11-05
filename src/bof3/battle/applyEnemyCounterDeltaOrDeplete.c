#include "bof3/battle/battle03_internal.h"

void func_800AAC88(s16 arg0, u8 arg1); /* @source 0x800AAC88 */

/* @source 0x801E496C
 * @behavior Applies the signed enemy-work halfword +0xF8 delta to the current
 * halfword +0x94: when the delta is not below the current value it stores 0
 * into +0x94 and 0x4000 into +0x82; otherwise it raises front-end cue 0x206
 * for a negative delta, subtracts the delta from +0x94 and lowers that result
 * to the +0xA0 ceiling. While enemy byte +0xFC has bit 0 set it then forwards
 * the delta and scratch byte +0x05 to func_800AAC88, runs enemyReadyOrHelper1
 * and advances scratch byte +0x03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyEnemyCounterDeltaOrDeplete(void)
{
  if (battleCurrentEnemyWorkState->unk_94 <=
      (s16)battleCurrentEnemyWorkState->unk_f8) {
    battleCurrentEnemyWorkState->unk_94 = 0;
    battleCurrentEnemyWorkState->unk_82 = 0x4000;
  } else {
    if ((s16)battleCurrentEnemyWorkState->unk_f8 < 0) {
      func_8015DF18(0x206);
    }
    battleCurrentEnemyWorkState->unk_94 =
        battleCurrentEnemyWorkState->unk_94 -
        battleCurrentEnemyWorkState->unk_f8;
    if (battleCurrentEnemyWorkState->unk_94 >
        battleCurrentEnemyWorkState->unk_a0) {
      battleCurrentEnemyWorkState->unk_94 = battleCurrentEnemyWorkState->unk_a0;
    }
  }
  if (battleCurrentEnemyWorkState->unk_fc & 1) {
    func_800AAC88((s16)battleCurrentEnemyWorkState->unk_f8,
                  D_1F800044->unk_05);
  }
  enemyReadyOrHelper1();
  D_1F800044->unk_03++;
}
