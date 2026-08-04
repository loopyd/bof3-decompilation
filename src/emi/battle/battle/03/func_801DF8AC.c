#include "internal.h"

/* @behavior selects the next local-state handler from one of two tables and calls
 * it immediately.
 * @source 0x801DF8AC
 */
void NO_SIBLING_CALLS func_801DF8AC(void) {
  Battle03Handler handler;

  if ((BATTLE_LOCAL_WORD_128(BATTLE_LOCAL_WORK_PTR) & 1u) != 0u) {
    handler = D_801EB188;
  } else {
    handler = D_801EB15C[BATTLE_LOCAL_BYTE_79(BATTLE_LOCAL_WORK_PTR)];
  }

  handler();
}
