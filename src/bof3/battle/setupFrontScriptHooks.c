#include "bof3/battle/battle03_internal.h"

/* @source 0x801E320C
 * @behavior Publishes D_801EB288 as the current enemy work's script pointer at
 * +0xEC, registers requestFrontSelectorState1 and enterFrontendMode
 * as the battle hooks at
 * 0x801463A4/0x801463A8, runs the enemy script byte helper stateDeriveScriptFlag for
 * class argument 0, raises bit 0x40 of scratch work byte +0x00, submits the
 * positional 0x80 effect for the work's flags halfword +0x82, then increments
 * scratch work byte +0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setupFrontScriptHooks(void) {
  FIELD_REF(volatile u8*, battleCurrentEnemyWork, 0xecu) = D_801EB288;
  D_801463A4 = requestFrontSelectorState1;
  D_801463A8 = enterFrontendMode;
  stateDeriveScriptFlag(0);
  ((u8*)D_1F800044)[0] |= 0x40;
  submitPositionalEffectBit80(BATTLE_ENEMY_FLAGS_82(battleCurrentEnemyWork));
  ((u8*)D_1F800044)[1]++;
}
