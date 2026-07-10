#include "internal.h"

/* does: finds one free COMMU00 source slot from a random start, copies its
 * 5-byte template seed into the active mirror, timestamps it, and appends the
 * slot id to the pending queue.
 * @source: 0x801f00d4 FUN_801f00d4
 */
void func_801f00d4(void) {
  volatile Commu00ActiveRecord* active_record;
  volatile u32*                 progress_counter;
  volatile u8*                  active_template_base;
  volatile u8*                  active_template;
  volatile u8*                  active_template_end;
  const volatile u8*            slot_template_table;
  const volatile u8*            template_row;
  u32                           source_index_u32;
  s32                           active_offset;
  u32                           progress_anchor;
  u8                            source_index;

  source_index = (u8)(game_random_u16() & 0x3fu);
  if (source_index > 0x3bu) {
    source_index = (u8)(source_index - 0x3cu);
  }

  progress_counter = (volatile u32*)0x8014502cu;
  active_template_base = (volatile u8*)((u32)progress_counter + 0x7bcu);
  slot_template_table = COMMU00_SLOT_TEMPLATE_TABLE;

  while (1) {
    source_index_u32 = (u32)source_index;
    active_offset = (s32)(source_index_u32 * 8u);
    if (((const volatile u8*)COMMU00_ACTIVE_RECORD_BASE)[active_offset] == 0u) {
      break;
    }

    source_index += 1u;
    if (source_index > 0x3bu) {
      source_index = 0u;
    }
  }

  active_record = (volatile Commu00ActiveRecord*)(COMMU00_ACTIVE_RECORD_BASE +
                                                  active_offset);
  active_template = active_template_base + (source_index_u32 * 5u);
  template_row = slot_template_table + (source_index_u32 * 9u);
  active_record->active = 1u;
  active_record->kind = 0u;
  progress_anchor = *progress_counter;
  active_record->record_state = 0u;
  active_record->progress_anchor = progress_anchor;

  active_template_end = active_template + 5u;
  do {
    *active_template = *template_row;
    active_template += 1;
    template_row += 1;
  } while (active_template < active_template_end);

  COMMU00_PENDING_QUEUE[COMMU00_PENDING_QUEUE_COUNT] = source_index;
  COMMU00_PENDING_QUEUE_COUNT += 1u;
}
