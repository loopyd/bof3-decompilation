#include "bof3/battle/battle15_internal.h"

/* @source 0x80097CBC
 * @behavior resets the battle selection state after queuing event 0x106.
 * Raw tables 0x800B4404/0x800B4414 begin with this void(void) handler.
 * battle/03 evidence establishes func_8015DF18(u16) and submitSlotScriptEvent(s32).
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueEventNoPanel(void) {
  func_8015DF18(0x106u);
  submitSlotScriptEvent((s32)BATTLE_ACTIVE_MESSAGE_SLOT_PTR);
  D_801462E1 = 4u;
  D_801462EF = 0u;
  D_801462E2 = 1u;
  D_801462E3 = 0u;
  D_801462E4 = 1u;
}
