#include "bof3/battle/battle03_internal.h"

/* @source 0x801E3638
 * @behavior Dispatches through the D_801EB3FC handler table by scratch work byte
 * +0x02, then -- while the current enemy work flags halfword +0x82 has bit 0x20
 * -- steps the slot-store countdown byte +0x09 of the record selected by scratch
 * work byte +0x05: increments it when enemy word +0x100 has bit 0x800, decrements
 * it otherwise, and toggles bit 0x800 once the signed count leaves [-11, 11]. It
 * then rebuilds the scratch work position from the selected slot record -- slot
 * word +0x34 plus the signed count shifted left 9 into work word +0x34 when
 * scratch work byte +0x08 is 1 or 3, otherwise the same expression from slot word
 * +0x38 into work word +0x38 -- and finally runs enemyReadyOrHelper1 while enemy
 * word +0x100 bit 0x1000 is clear.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E3638(void) {
  Battle03LocalWork* work;
  u8                 count;
  u8                 index;

  D_801EB3FC[battleWork[2]]();
  if ((BATTLE_ENEMY_FLAGS_82(battleCurrentEnemyWorkState) & 0x20u) != 0u) {
    if ((BATTLE_ENEMY_WORD_100(battleCurrentEnemyWorkState) & 0x800u) != 0u) {
      index = battleWork[5];
      count = D_801EC330[index].flag_09 + 1;
      D_801EC330[index].flag_09 = count;
      if ((s8)count >= 0xc) {
        BATTLE_ENEMY_WORD_100(battleCurrentEnemyWorkState) ^= 0x800u;
      }
    } else {
      index = battleWork[5];
      count = D_801EC330[index].flag_09 - 1;
      D_801EC330[index].flag_09 = count;
      if ((s8)count < -0xb) {
        BATTLE_ENEMY_WORD_100(battleCurrentEnemyWorkState) ^= 0x800u;
      }
    }
    work = (Battle03LocalWork*)battleWork;
    if (work->unk_08 == 1u || work->unk_08 == 3u) {
      index = work->unk_05;
      work->unk_34 =
          D_801EC330[index].unk_34 + (((s8)D_801EC330[index].flag_09) << 9);
    } else {
      index = work->unk_05;
      work->unk_38 =
          D_801EC330[index].unk_38 + (((s8)D_801EC330[index].flag_09) << 9);
    }
  }
  if ((BATTLE_ENEMY_WORD_100(battleCurrentEnemyWorkState) & 0x1000u) == 0u) {
    enemyReadyOrHelper1();
  }
}
