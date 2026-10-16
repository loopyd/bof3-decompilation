#include "bof3/battle/battle03_internal.h"

/* @behavior overwrites one event-queue slot directly from the caller's parameters
 * and marks the slot active.
 * @source 0x801DE60C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DE60C(u32 arg0, u8 arg1, u8 arg2, u8 arg3, u8 arg4, u32 arg5) {
  u8 flag;
  u32 offset;
  u16 halfword;

  arg0 &= 0xffu;
  offset = arg0 * 0xcu;
  halfword = arg4;
  flag = battleDispatchSlots[offset];
  battleDispatchSlots[offset + 1u] = arg1;
  battleDispatchSlots[offset + 2u] = arg2;
  battleDispatchSlots[offset + 3u] = arg3;
  battleDispatchSlots[offset + 10u] = 0u;
  battleDispatchSlots[offset] = flag | 1u;
  *(volatile u16*)&battleDispatchSlots[offset + 8u] = halfword;
  *(volatile u32*)&battleDispatchSlots[offset + 4u] = arg5;
}
