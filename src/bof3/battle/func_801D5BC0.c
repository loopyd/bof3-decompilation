#include "bof3/battle/battle03_internal.h"

/* @behavior resets one local/enemy `0x80` path, zeroes saved values, rebuilds the
 * followup state bytes, and queues the common event when needed.
 * @source 0x801D5BC0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 func_801D5BC0(void) {
  u8 index;
  u8 count;

  count = 0u;
  index = 0u;
  do {
    if (func_801D64C4(index) == 0u) {
      if ((D_80145E90[index].unk_80 & 0x80u) != 0u) {
        D_80145E90[index].unk_11e = 0u;
        D_80145E90[index].unk_11c = 0u;
        func_801DCEF8(index);
        count += 1u;
        D_80145E90[index].unk_01 = 6u;
        D_80145E90[index].unk_02 = 5u;
        D_80145E90[index].unk_04 = 0u;
        D_80145E90[index].unk_03 = 0u;
        D_80145E90[index].unk_120 = 0x11u;
        markPendingBit(index);
      }
    }
    index += 1u;
  } while (index < 3u);

  index = 3u;
  do {
    u32 slot;

    if (func_801D64C4(index) == 0u) {
      slot = index - 3u;
      if ((D_801EB630[slot].unk_82 & 0x80u) != 0u) {
        D_801EB630[slot].unk_fa = 0u;
        D_801EB630[slot].unk_f8 = 0u;
        func_801DCEF8(index);
        count += 1u;
        D_801EB630[slot].unk_01 = 6u;
        D_801EB630[slot].unk_02 = 5u;
        D_801EB630[slot].unk_04 = 0u;
        D_801EB630[slot].unk_03 = 0u;
        D_801EB630[slot].unk_fc = 0x11u;
        markPendingBit(index);
      }
    }
    index += 1u;
  } while (index < 0x0bu);

  if (count != 0u) {
    u32 event_id;

    initUiBundleSlot0();
    event_id = func_801502D0(0x18u);
    func_801DE560(2u, 0u, 0u, 0x2du, event_id);
    return 1u;
  }

  return 0u;
}
