#include "internal.h"

#define COMMU00_FAIRY_GIFTS CVPTR(u8, 0x801eec48u)

extern void      func_80150224(u32 message_id);
extern const u8* func_80165d48(u8 item_type, u8 item_index);
extern u8        func_801650b4(u8 item_type, u8 item_index, u8 mode, u8 arg3);

/* @behavior selects the next fairy gift from the battle-progression table,
 * displays the item name, updates the active fairy record, and advances the
 * COMMU00 progression state.
 * @source 0x801f18f8 FUN_801f18f8
 * @see docs/specs/data/fairies.md
 */
void func_801f18f8(void) {
  u8                            gift_rows[20 * 4];
  const volatile u8*            source;
  const u8*                     item_name;
  volatile Commu00ActiveRecord* active;
  u32                           battle_count;
  u32                           progress_anchor;
  u32                           row_index;
  u32                           byte_index;
  u8                            fairy_index;
  u8                            item_index;
  u8                            item_type;

  source = COMMU00_FAIRY_GIFTS;
  byte_index = 0u;
  do {
    gift_rows[byte_index] = source[byte_index];
    byte_index++;
  } while (byte_index < sizeof(gift_rows));

  fairy_index = COMMU00_FAIRY_SLOT_INDEX[0];
  active = commu00_mutable_active_record(fairy_index);
  battle_count = COMMU00_BATTLE_COUNT[0];
  progress_anchor = active->progress_anchor;
  row_index = 0u;
  while (row_index < 20u && battle_count - progress_anchor >=
                                *(const u16*)&gift_rows[row_index * 4u]) {
    row_index++;
  }

  if (row_index == 0u) {
    func_80150224(0x98u);
  } else {
    row_index--;
    item_index = gift_rows[row_index * 4u + 2u];
    item_type = gift_rows[row_index * 4u + 3u];
    item_name = func_80165d48(item_type, item_index);
    byte_index = 0u;
    do {
      COMMU00_ITEM_NAME[byte_index] = item_name[byte_index];
      byte_index++;
    } while (byte_index < 12u);
    COMMU00_ITEM_NAME[12] = 0u;
    func_80150224(0x96u);

    if (func_801650b4(item_type, item_index, 1u, 0u) == 0u) {
      COMMU00_FAIRY_PROGRESS[0]++;
    } else {
      active->progress_anchor = battle_count;
    }
  }

  COMMU00_STATE[0] = 2u;
  COMMU00_FAIRY_PROGRESS[0]++;
}
