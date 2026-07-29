#include "internal.h"

/* @source 0x800982E4
 * @behavior queues event 0x106, stages the active message slot, and resets
 * the battle selection state.
 */
void func_800982E4(void) {
  func_8015DF18(0x106u);
  D_8014839F = 1u;
  D_8014837B = 1u;
  func_801DEA64((s32)BATTLE_ACTIVE_MESSAGE_SLOT_PTR);
  D_801462EF = 0u;
  D_801462E1 = 2u;
  D_801462E2 = 0u;
  D_801462E3 = 0u;
}
