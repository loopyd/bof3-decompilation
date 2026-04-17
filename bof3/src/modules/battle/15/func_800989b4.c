#include "internal.h"

/* does: once the secondary grid gate clears, restores the battle owner tuple
 * to the shared selection root and reapplies the local panel-task band reset
 * helper.
 * @source: 0x800989b4 FUN_800989b4
 */
void BOF3_NO_SIBLING_CALLS func_800989b4(void) {
  if (BOF3_BATTLE_SELECTION_LOCKED != 0u) {
    return;
  }

  BOF3_BATTLE_SELECTION_PHASE = 1u;
  BOF3_BATTLE_SELECTION_OWNER_STATE = 0u;
  BOF3_BATTLE_SELECTION_ROOT_STATE = 0u;
  func_8009b20c();
}
