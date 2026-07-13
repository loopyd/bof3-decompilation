#include "internal.h"

#define COMMU00_EXPLORE_THRESHOLDS VPTR(u8, 0x801457a9u)
#define COMMU00_EXPLORE_ITEMS      CVPTR(u8, 0x801f2618u)

extern void      func_80150224(u32 message_id);
extern u16       func_8017e3d4(void);
extern const u8* func_80165d48(u8 item_type, u8 item_index);
extern u8        func_801650b4(u8 item_type, u8 item_index, u8 mode, u8 arg3);

/* @behavior resolves a fairy exploration reward from the active progression
 * state, checks the two-byte exploration-item table, displays the reward when
 * valid, and advances the COMMU00 state machine.
 * @source 0x801f1bc8 FUN_801f1bc8
 * @see docs/specs/data/fairies.md
 */
void func_801f1bc8(void) {
  u8                            local_table[12];
  const volatile u8*            source;
  const volatile u8*            thresholds;
  const u8*                     item_name;
  volatile Commu00ActiveRecord* active;
  u32                           byte_index;
  u32                           random_value;
  u32                           remaining;
  u32                           table_index;
  u32                           row_index;
  u8                            state;
  u8                            item_index;
  u8                            item_type;
  u8                            reward_class;
  u8                            fairy_index;

  source = CVPTR(u8, 0x801eec98u);
  byte_index = 0u;
  do {
    local_table[byte_index] = source[byte_index];
    byte_index++;
  } while (byte_index < sizeof(local_table));

  fairy_index = COMMU00_FAIRY_SLOT_INDEX[0];
  active = commu00_mutable_active_record(fairy_index);
  state = active->record_state;
  if (state == 3u) {
    func_80150224(0x52u);
    active->progress_anchor = COMMU00_BATTLE_COUNT[0];
    reward_class = 0u;
    active->record_state = reward_class;
    COMMU00_ACTIVE_UI[0x7cu] = 0x51u;
  } else if (state == 2u) {
    random_value = (u32)(func_8017e3d4() & 0x7fu);
    if (random_value >= 0x65u) {
      random_value = 0x64u;
    }

    table_index = (u32)active->kind - 1u;
    thresholds = COMMU00_EXPLORE_THRESHOLDS + table_index * 8u;
    remaining = random_value - local_table[table_index * 4u + 1u];
    if (random_value < local_table[table_index * 4u + 1u]) {
      remaining = 0u;
    }

    reward_class = 1u;
    while (remaining >= thresholds[reward_class - 1u] && reward_class < 3u) {
      remaining -= thresholds[reward_class - 1u];
      reward_class++;
    }

    row_index =
        ((u32)(reward_class - 1u) * 0x10u + (u32)(func_8017e3d4() & 0x0fu)) *
        2u;
    item_index = COMMU00_EXPLORE_ITEMS[row_index];
    item_type = COMMU00_EXPLORE_ITEMS[row_index + 1u];
    if (func_801650b4(item_type, item_index, 1u, 0u) == 0u) {
      func_80150224(0x50u);
    } else {
      item_name = func_80165d48(item_type, item_index);
      byte_index = 0u;
      do {
        COMMU00_ITEM_NAME[byte_index] = item_name[byte_index];
        byte_index++;
      } while (byte_index < 12u);
      COMMU00_ITEM_NAME[12] = 0u;
      func_80150224(0x4fu);
      active->progress_anchor = COMMU00_BATTLE_COUNT[0];
    }

    active->record_state = 0u;
    COMMU00_ACTIVE_UI[0x7cu] = 0x51u;
  }

  COMMU00_STATE[0] = 2u;
  COMMU00_FAIRY_PROGRESS[0]++;
}
