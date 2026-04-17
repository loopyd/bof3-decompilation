#include "internal.h"

/* does: finds the first free event slot in the eight-entry event queue and
 * populates it with the caller's parameters.
 * @source: 0x801de560 FUN_801de560
 */
void func_801de560(u8 arg0, u8 arg1, u8 arg2, u8 arg3, u32 arg4) {
  u8  index;
  u8  value;
  u32 offset;

  index = 2u;
  while (index < 8u) {
    offset = (u32)index * 0xcu;
    value = *(volatile u8*)(0x801eb4f0u + offset);
    if (value == 0u) {
      *(volatile u8*)(0x801eb4f0u + offset) = value | 1u;
      *(volatile u8*)(0x801eb4f1u + offset) = arg0;
      *(volatile u8*)(0x801eb4f2u + offset) = arg1;
      *(volatile u8*)(0x801eb4f3u + offset) = arg2;
      *(volatile u16*)(0x801eb4f8u + offset) = arg3;
      *(volatile u32*)(0x801eb4f4u + offset) = arg4;
      *(volatile u8*)(0x801eb4fau + offset) = 0u;
      return;
    }
    index += 1u;
  }
}
