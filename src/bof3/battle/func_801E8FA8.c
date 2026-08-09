#include "bof3/battle/battle03_internal.h"

/* @behavior chooses the next enabled bit from the current target's `0x58` mask,
 * starting just after the scratch byte `0x0b` and wrapping modulo 16.
 * @source 0x801E8FA8
 * @status partial
 * @match 59.62
 * @residual non-exact live audit: 31/51 instructions; 204 original bytes versus 208 current.
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
    mask = BATTLE_LOCAL_ABS_HALF_5F10(kind);
  } else {
    mask = BATTLE_ENEMY_ABS_HALF_6B2(((u32)kind - 3u) & 0xffu);
  }

  start = BATTLE_SCRATCH_CELL_U8PTR[0xb];
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

  return BATTLE_SCRATCH_CELL_U8PTR[0xb];
}
