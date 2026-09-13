#include "bof3/battle/battle03_internal.h"

/* @behavior resolves one event id from the `0x801eb09c` script table and queues the
 * standard event packet that plays it.
 * @source 0x801DEBC4
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void queueScriptEvent(u32 arg0, u32 arg1) {
  const u16* event_row;
  const u16* event_slot;
  const u16* event_table;
  u32        event_slot_offset;
  u32        event_row_index;
  u16        event_id;

  event_row_index = arg0 & 0xffu;
  event_table = D_801EB09C;
  event_row = &event_table[event_row_index * 10u];
  event_slot_offset = (arg1 & 0xffu) * 2u;
  event_slot = (const u16*)(event_slot_offset + (u32)event_row);

  event_id = *event_slot;
  func_801DE560(2u, 0u, 0u, 0x2du, func_801502D0(event_id));
}
