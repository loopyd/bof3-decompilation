#include "internal.h"

/* @behavior appends one triple `(byte, byte, word)` into the 16-entry UI ring and
 * advances the ring tail.
 * @source 0x801de8c0 FUN_801de8c0
 */
void func_801de8c0(s8 arg0, s8 arg1, u32 arg2) {
  u8 index;

  *(volatile u8*)(0x801f0000u +
                  ((u32)(*(volatile u8*)(0x801f0000u - 0x3cd8u)) << 3) -
                  0x4a50u) = (u8)arg0;
  *(volatile u8*)(0x801f0000u +
                  ((u32)(*(volatile u8*)(0x801f0000u - 0x3cd8u)) << 3) -
                  0x4a4fu) = (u8)arg1;
  index = *(volatile u8*)(0x801f0000u - 0x3cd8u);
  *(volatile u32*)(0x801f0000u + ((u32)index << 3) - 0x4a4cu) = arg2;
  *(volatile u8*)(0x801f0000u - 0x3cd8u) = (index + 1u) & 0x0fu;
}
