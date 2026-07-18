#include "internal.h"

/* @behavior dispatches the current local work record through the state table when
 * the active flag bit is set.
 * @source 0x801DEEB4
 */
void func_801DEEB4(void) {
  if ((BATTLE_LOCAL_SCRATCH_PTR->flags_00 & 1u) != 0u) {
    BATTLE_LOCAL_STATE_TABLE[BATTLE_LOCAL_SCRATCH_PTR->unk_01]();
  }
}
