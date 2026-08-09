#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current local work record through the state table when
 * the active flag bit is set.
 * @source 0x801DEEB4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchLocalStateTable(void) {
  if ((BATTLE_LOCAL_SCRATCH_PTR->flags_00 & 1u) != 0u) {
    D_801EB120[BATTLE_LOCAL_SCRATCH_PTR->unk_01]();
  }
}
