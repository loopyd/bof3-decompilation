#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4E10
 * @behavior While the current enemy work mode byte 0xf5 is 4, folds its 0x96
 * halfword down by the global 0x801463c8 byte, raises global bit 0x800, clears
 * the mode byte, calls the enemy-ready helper unless the selected kind mask
 * already carries bit 0x800, and advances scratchpad work byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E4E10(void) {
  u16* flags;
  u16 index;

  if (battleCurrentEnemyWorkState->unk_f5 == 4u) {
    FIELD_REF(u16, battleCurrentEnemyWorkState, 0x96) -= BATTLE_GLOBAL_BYTE_63C8;
  }
  flags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
  *flags |= 0x800u;
  battleCurrentEnemyWorkState->unk_f5 = 0u;
  index = BATTLE_GLOBAL_HALF_63C0;
  if ((D_801CA71C[index].mask_00 & 0x800u) == 0u) {
    enemyReadyOrHelper2();
  }
  D_1F800044->unk_02++;
}
