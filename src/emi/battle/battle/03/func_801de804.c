#include "internal.h"

/* @behavior clears the first three bytes of all eight event-queue slots.
 * @source 0x801de804 FUN_801de804
 */
void func_801de804(void) {
  u8  index;
  u32 offset;

  index = 0u;
  do {
    offset = (u32)index * 0xcu;
    *(volatile u8*)(0x801eb4f0u + offset) = 0u;
    *(volatile u8*)(0x801eb4f1u + offset) = 0u;
    *(volatile u8*)(0x801eb4f2u + offset) = 0u;
    index += 1u;
  } while (index < 8u);
}
