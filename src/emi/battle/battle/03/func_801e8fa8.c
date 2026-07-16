#include "internal.h"

/* @behavior chooses the next enabled bit from the current target's `0x58` mask,
 * starting just after the scratch byte `0x0b` and wrapping modulo 16.
 * @source 0x801E8FA8
 */
u8 func_801E8FA8(void) {
  volatile u8* slot;
  u16          mask;
  u8           index;
  u8           start;
  u32          kind;
  u32          offset;

  slot = BATTLE_CURRENT_QUEUED_PTR_4B20;
  kind = slot[5];

  if (kind < 3u) {
    offset = (u32)kind;
    offset = offset * 0x140u;
    mask = *(volatile u16*)(0x80145f10u + offset);
  } else {
    offset = (((u32)kind - 3u) & 0xffu) * 0x118u;
    mask = *(volatile u16*)(0x801eb6b2u + offset);
  }

  start = (*(volatile u8**)0x1f800044u)[0xb];
  mask &= 0x58u;
  index = ((u32)start + 1u) & 0x0fu;
  if (index != (u32)start) {
    do {
      if ((((s32)mask) >> index & 1U) != 0) {
        return (u8)index;
      }
      index = (index + 1u) & 0x0fu;
    } while (index != (u32)start);
  }

  return (*(volatile u8**)0x1f800044u)[0xb];
}
