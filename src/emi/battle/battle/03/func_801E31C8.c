#include "internal.h"

/* @behavior dispatches the current local presentation state-1 byte through its
 * table.
 * @source 0x801E31C8
 */
void NO_SIBLING_CALLS func_801E31C8(void) {
  volatile u8*     scratch;
  Battle03Handler* table;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  table = (Battle03Handler*)BATTLE_LOCAL_PRESENTATION_STATE1_TABLE;
  table[scratch[1]]();
}
