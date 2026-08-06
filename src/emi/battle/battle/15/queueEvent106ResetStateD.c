#include "internal.h"

/* @source 0x80099114
 * @behavior queues event 0x106, stages the active message slot, and resets
 * the battle selection state.
 */
void queueEvent106ResetStateD(void) {
  func_8015DF18(0x106u);
  func_801DEA64((s32)BATTLE_ACTIVE_MESSAGE_SLOT_PTR);
  D_801462E1 = 4u;
  D_801462E2 = 2u;
  D_801462EF = 0u;
  D_801462E3 = 0u;
  D_801462E4 = 1u;
}
