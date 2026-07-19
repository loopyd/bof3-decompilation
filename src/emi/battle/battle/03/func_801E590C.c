#include "internal.h"

/* @behavior finds the first free queued-slot entry, marks it active, and stores the
 * caller's pair of mode bytes into offsets `5/6`.
 * @source 0x801E590C
 */
u32 func_801E590C(u32 arg0, u32 arg1) {
  u8  index;
  u8  flags;
  u32 offset;

  index = 0u;
  while (index < 0x30u) {
    offset = (u32)index * 0x78u;
    flags = BATTLE_SLOT_STORE_BYTE_00(index);
    if ((flags & 1u) == 0u) {
      flags = (u8)(flags | 1u);
      BATTLE_SLOT_STORE_BYTE_00(index) = flags;
      BATTLE_SLOT_STORE_BYTE_06(index) = (u8)arg0;
      BATTLE_SLOT_STORE_BYTE_05(index) = (u8)arg1;
      return index;
    }
    index += 1u;
  }
  return 0xffu;
}
