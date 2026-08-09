#include "bof3/battle/battle03_internal.h"

/* @behavior resolves one event id from the `0x801eb09c` script table and queues the
 * standard event packet that plays it.
 * @source 0x801DEBC4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
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
  /*
   * MATCHING_AID:
   * The original stores $ra to the stack before the table `lhu` (asm-diff:
   * original `sw $ra,0x18($sp)` precedes `lhu $a0,0($a1)`; current schedules
   * the load before the prologue store). This barrier pins the original
   * store-then-load memory ordering. Remove if the compiler's prologue
   * scheduling is understood. bin/byte-match was exact with this aid.
   */
  barrier();
  event_id = *event_slot;
  func_801DE560(2u, 0u, 0u, 0x2du, func_801502D0(event_id));
}
