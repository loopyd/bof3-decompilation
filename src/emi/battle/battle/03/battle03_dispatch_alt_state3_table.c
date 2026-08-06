#include "internal.h"

/* @behavior dispatches the current alternate local state-3 byte through its table.
 * @source 0x801E1450
 */
void NO_SIBLING_CALLS battle03_dispatch_alt_state3_table(void) {
  volatile Battle03LocalWork* work;

  work = BATTLE_LOCAL_SCRATCH_PTR;
  D_801EB218[work->unk_03]();
}
