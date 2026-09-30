#include "bof3/battle/battle03_internal.h"

/* @source 0x801E3B04
 * @behavior When enemyReadyOrHelper1 reports ready, forwards the battle byte
 * D_80146384 to func_800A9820, runs the 0x80 slot allocation, raises bit 0x4 of
 * the battle global halfword BATTLE_GLOBAL_HALF_62E8 and resets the enemy
 * scratch state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E3B04(void) {
  if (enemyReadyOrHelper1() != 0) {
    func_800A9820(D_80146384);
    raiseBit80AllocSlot();
    BATTLE_GLOBAL_HALF_62E8 |= 0x4;
    resetEnemyScratchState();
  }
}
