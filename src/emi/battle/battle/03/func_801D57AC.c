#include "internal.h"

/* @behavior advances one enemy `0x40` countdown path, clearing the flag and queuing
 * a followup event once one or more battlers complete.
 * @source 0x801D57AC
 */
u8 func_801D57AC(void) {
  u8 index;
  u8 count;
  u8 last_index;

  count = 0u;
  last_index = 0u;
  index = 3u;
  do {
    u32 slot;

    if (func_801D64C4(index) == 0u) {
      slot = index - 3u;
      if ((D_801EB630[slot].unk_82 & 0x40u) != 0u) {
        if (func_801DDCB4(index) != 0u) {
          u16 flags;

          last_index = index;
          flags = D_801EB630[slot].unk_82;
          D_801EB630[slot].unk_fd = 0u;
          D_801EB630[slot].unk_82 = flags & 0xffbfu;
          count += 1u;
        } else {
          D_801EB630[slot].unk_fd += 1u;
        }
      }
    }
    index += 1u;
  } while (index < 0x0bu);

  if (count != 0u) {
    u32 event_id;

    initUiBundleSlot0();
    event_id = 0x27u;
    if (count == 1u) {
      submitEnemyScriptBlock(last_index);
      event_id = 0x25u;
    }
    event_id = func_801502D0(event_id);
    func_801DE560(2u, 0u, 0u, 0x2du, event_id);
    return 1u;
  }

  return 0u;
}
