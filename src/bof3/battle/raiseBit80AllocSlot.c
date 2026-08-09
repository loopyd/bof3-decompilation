#include "bof3/battle/battle03_internal.h"

extern int rand(void);
/* @behavior conditionally raises the global `0x80` bit and allocates one queued
 * slot when the current enemy scratch flags allow it.
 * @source 0x801E5704
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void raiseBit80AllocSlot(void) {
  u16* flags;

  if (((BATTLE_ENEMY_FLAGS_80(BATTLE_CURRENT_ENEMY_PTR) & 8u) != 0u) &&
      ((rand() & 1u) != 0u)) {
    flags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
    *flags |= 0x80u;
    func_801E590C(0u, 2u);
  }
}
