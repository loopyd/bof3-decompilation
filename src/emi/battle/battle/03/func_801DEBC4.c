#include "internal.h"

/* @behavior resolves one event id from the `0x801eb09c` script table and queues the
 * standard event packet that plays it.
 * @source 0x801DEBC4
 */
void func_801DEBC4(u32 arg0, u32 arg1) {
  const volatile u16* event_row;
  u16                 event_id;

  event_row = &BATTLE_EVENT_SCRIPT_TABLE_B09C[(arg0 & 0xffu) * 10u];
  event_id = event_row[arg1 & 0xffu];
  func_801DE560(2u, 0u, 0u, 0x2du, func_801502D0(event_id));
}
