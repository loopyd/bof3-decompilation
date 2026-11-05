#include "bof3/battle/battle03_internal.h"

extern u32 D_8014632C; /* @source 0x8014632C @kind data */
extern u32 D_80146330; /* @source 0x80146330 @kind data */
extern u8 battleByte801463ca; /* @source 0x801463CA @kind unknown */

/* @source 0x801E54EC
 * @behavior Raises scratch work flag 0x40, adds the current enemy work
 *           halfwords +0x86 and +0x84 into the global accumulators D_8014632C
 *           and D_80146330, sets enemy halfword +0x82 bit 0x4000 and clears
 *           +0x84, clears the selected battler's action flags, runs the random
 *           enemy entry collection, decrements the D_801462F3 counter and
 *           raises D_80146328 bit 1 when it reaches zero, clears global
 *           halfword 62E8 bit 0x4000 while the panel byte still names the same
 *           battler, runs func_800AD074 on enemy byte +0x7C, submits the
 *           scratch work through func_80196718, clears enemy word +0x100 bits
 *           0x70 and the pending bit, then re-arms scratch bytes +1..+3 as
 *           2/0/0. Reached from applyGravityOrReset once the scratch vertical
 *           field +0x44 turns negative.
 * @status partial
 * @match 99.00
 * @residual The +0x84 zero store precedes the scratch-pointer cell load;
 *           the original places it after that load. All other instructions
 *           match, with 99 of 100 matched at the original 400 bytes.
 */
void accumulateEnemyHalfwordsAndResetScratch(void) {
  Battle03EnemyWork *enemy;
  u32 *accumulator;
  u16 flags;
  u16 half;

  D_1F800044->flags_00 |= 0x40u;
  enemy = battleCurrentEnemyWorkState;
  accumulator = &D_8014632C;
  *accumulator += enemy->unk_86;
  D_80146330 += enemy->unk_84;
  half = enemy->unk_82;
  BATTLE_ENEMY_HALF_84(enemy) = 0u;
  enemy->unk_82 = half | 0x4000u;
  clearBattlerActionFlags(D_1F800044->unk_05);
  collectRandomEnemyEntries_battle03_801E531C();
  if (--D_801462F3 == 0) {
    D_80146328 |= 2;
  }
  flags = BATTLE_GLOBAL_HALF_62E8;
  if ((flags & 0x4000u) != 0u) {
    if (battleByte801463ca == D_1F800044->unk_05) {
      BATTLE_GLOBAL_HALF_62E8 = flags & 0xbfffu;
    }
  }
  func_800AD074(BATTLE_ENEMY_BYTE_7C(battleCurrentEnemyWorkState));
  func_80196718(D_1F800044);
  battleCurrentEnemyWorkState->unk_100 &= 0xffffff8fu;
  clearPendingBit(D_1F800044->unk_05);
  D_1F800044->unk_01 = 2u;
  D_1F800044->unk_02 = 0u;
  D_1F800044->unk_03 = 0u;
}
