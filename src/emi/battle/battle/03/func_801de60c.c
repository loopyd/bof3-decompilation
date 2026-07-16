#include "internal.h"

/* @behavior overwrites one event-queue slot directly from the caller's parameters
 * and marks the slot active.
 * @source 0x801DE60C
 */
void func_801DE60C(u32 arg0, u8 arg1, u8 arg2, u8 arg3, u8 arg4, u32 arg5) {
  u32 offset;
  u8  flag;

  arg0 &= 0xffu;
  offset = arg0 * 0xcu;
  flag = ((volatile u8*)0x801f0000u)[offset - 0x4b10u];
  ((volatile u8*)0x801f0000u)[offset - 0x4b0fu] = arg1;
  ((volatile u8*)0x801f0000u)[offset - 0x4b0eu] = arg2;
  ((volatile u8*)0x801f0000u)[offset - 0x4b0du] = arg3;
  ((volatile u8*)0x801f0000u)[offset - 0x4b06u] = 0u;
  ((volatile u8*)0x801f0000u)[offset - 0x4b10u] = flag | 1u;
  *(volatile u16*)((volatile u8*)0x801f0000u + offset - 0x4b08u) = arg4;
  *(volatile u32*)((volatile u8*)0x801f0000u + offset - 0x4b0cu) = arg5;
}
