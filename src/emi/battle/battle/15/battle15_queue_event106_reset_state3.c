#include "internal.h"

/* @source 0x80097CBC
 * @behavior resets the battle selection state after queuing event 0x106.
 * Raw tables 0x800B4404/0x800B4414 begin with this void(void) handler.
 * battle/03 evidence establishes func_8015DF18(u16) and func_801DEA64(s32).
 */
void battle15_queue_event106_reset_state3(void) {
  func_8015DF18(0x106u);
  func_801DEA64((s32)BATTLE_ACTIVE_MESSAGE_SLOT_PTR);
  D_801462E1 = 4u;
  D_801462EF = 0u;
  D_801462E2 = 1u;
  D_801462E3 = 0u;
  D_801462E4 = 1u;
}
