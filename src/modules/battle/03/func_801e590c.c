#include "internal.h"

/* @behavior finds the first free queued-slot entry, marks it active, and stores the
 * caller's pair of mode bytes into offsets `5/6`.
 * @source 0x801e590c FUN_801e590c
 */
u32 func_801e590c(u32 arg0, u32 arg1) {
  u8  index;
  u8  flags;
  u32 offset;

  index = 0u;
  while (index < 0x30u) {
    offset = (u32)index * 0x78u;
    flags = *(volatile u8*)(0x801ec330u + offset);
    if ((flags & 1u) == 0u) {
      flags = (u8)(flags | 1u);
      *(volatile u8*)(0x801ec330u + offset) = flags;
      *(volatile u8*)(0x801ec336u + offset) = (u8)arg0;
      *(volatile u8*)(0x801ec335u + offset) = (u8)arg1;
      return index;
    }
    index += 1u;
  }
  return 0xffu;
}
