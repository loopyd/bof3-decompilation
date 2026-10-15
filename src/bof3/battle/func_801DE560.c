#include "bof3/battle/battle03_internal.h"

/* @behavior finds the first free event slot among queue slots 2 through 7 and
 * populates it with the caller's parameters.
 * @source 0x801DE560
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DE560(u8 arg0, u8 arg1, u8 arg2, u8 arg3, u32 arg4) {
  u8  index;
  u8  value;
  u32 offset;

  index = 2u;
  while (index < 8u) {
    offset = (u32)index * 0xcu;
    value = battleDispatchSlots[offset];
    if (value == 0u) {
      battleDispatchSlots[offset] = value | 1u;
      battleDispatchSlots[offset + 1u] = arg0;
      battleDispatchSlots[offset + 2u] = arg1;
      battleDispatchSlots[offset + 3u] = arg2;
      *(volatile u16*)&battleDispatchSlots[offset + 8u] = arg3;
      *(volatile u32*)&battleDispatchSlots[offset + 4u] = arg4;
      battleDispatchSlots[offset + 10u] = 0u;
      return;
    }
    index += 1u;
  }
}
