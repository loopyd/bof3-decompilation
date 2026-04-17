#include "internal.h"

/* does: resolves one event id from the `0x801eb09c` script table and queues the
 * standard event packet that plays it.
 * @source: 0x801debc4 FUN_801debc4
 */
void func_801debc4(u32 arg0, u32 arg1) {
  const volatile u16* event_row;
  u16                 event_id;

  event_row = &BOF3_BATTLE_EVENT_SCRIPT_TABLE_B09C[(arg0 & 0xffu) * 10u];
  event_id = event_row[arg1 & 0xffu];
  func_801de560(2u, 0u, 0u, 0x2du, func_801502d0(event_id));
}
