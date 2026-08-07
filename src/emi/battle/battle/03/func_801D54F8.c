#include "internal.h"

/* @behavior advances one local state-0x800 countdown path, restores saved local
 * coordinates when it completes, and emits one followup event when any battler
 * finished.
 * @source 0x801D54F8
 */
u8 func_801D54F8(void) {
  u8 index;
  u8 count;
  u8 last_index;

  count = 0u;
  last_index = 0u;
  index = 0u;
  do {
    if ((func_801D64C4(index) == 0u) &&
        ((D_80145E90[index].unk_80 & 0x0800u) != 0u) &&
        (D_80145E90[index].unk_137 == 5u)) {
      last_index = index;
      count += 1u;
      D_80145E90[index].unk_137 = 0u;
      D_80145E90[index].unk_88 = D_80145E90[index].unk_90;
      D_80145E90[index].unk_8a = D_80145E90[index].unk_92;
      D_80145E90[index].unk_8c = D_80145E90[index].unk_9e;
      func_800A36F0(index, 0x8ffu);
    }
    index += 1u;
  } while (index < 3u);

  if (count != 0u) {
    u32 event_id;

    initUiBundleSlot0();
    event_id = 0x2cu;
    if (count == 1u) {
      func_801DE9A8(last_index);
      event_id = 0x2bu;
    }
    event_id = func_801502D0(event_id);
    func_801DE560(2u, 0u, 0u, 0x2du, event_id);
    return 1u;
  }

  return 0u;
}
