#include "bof3/battle/battle03_internal.h"

/* @behavior chooses the next enabled bit from the current target's `0x58` mask,
 * starting just after the scratch byte `0x0b` and wrapping modulo 16.
 * @source 0x801E8FA8
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801E8FA8(void) {
  Battle03LocalWork* work;
  u16                mask;
  u8                 start;
  u8                 index;
  u8                 selector;

  work = D_801EB4E0;
  selector = work->unk_05;
  if (selector < 3u) {
    mask = D_80145E90[selector].unk_80;
  } else {
    mask = D_801EB630[selector - 3u].unk_82;
  }

  start = BATTLE_SCRATCH_CELL_U8PTR[0xb];
  mask &= 0x58u;
  index = (start + 1u) & 0x0fu;
  if (index != start) {
    do {
      if (((mask >> index) & 1u) != 0u) {
        return index;
      }
      index = (index + 1u) & 0x0fu;
    } while (index != start);
  }

  return BATTLE_SCRATCH_CELL_U8PTR[0xb];
}
