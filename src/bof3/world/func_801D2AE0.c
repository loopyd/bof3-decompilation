#include "bof3/world/area03004_internal.h"

/* Per-evaluation reload of the scratch cursor cell: the original emits
 * lui $v1,0x1f80 / lw $v1,0x44($v1) before each store group. */
#define AREA030_SCRATCH_CURSOR ((volatile u8*)D_1F800044)

/* @behavior either advances the local AREA030 scratch record from the shared world
 * state or falls back to the game-side helper when the countdown gate is not
 * active.
 * @source 0x801D2AE0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D2AE0(void) {
  volatile u8* scratch;
  volatile u8* world;

  world = (volatile u8*)0x80140000u;

  if ((*(s16*)(world + 0x4006u) < 1) || (world[0x3fc9u] == 0u)) {
    func_80196070();
    return;
  }

  scratch = AREA030_SCRATCH_CURSOR;
  *(volatile u32*)(scratch + 0x34u) = *(volatile u32*)(world + 0x3ffcu);
  *(volatile u32*)(scratch + 0x38u) = *(volatile u32*)(world + 0x4000u);
  scratch[0x2au] = world[0x3ff2u];

  scratch = AREA030_SCRATCH_CURSOR;
  scratch[0x49u] = world[0x4011u];
  scratch = AREA030_SCRATCH_CURSOR;
  scratch[0x4au] = world[0x4012u];
  scratch = AREA030_SCRATCH_CURSOR;
  scratch[0x4bu] = world[0x4013u];
  scratch = AREA030_SCRATCH_CURSOR;
  *(volatile u32*)(scratch + 0x50u) = *(volatile u32*)(world + 0x4018u);
  *(volatile u32*)(scratch + 0x54u) = *(volatile u32*)(world + 0x401cu);
  *(volatile u16*)(scratch + 0x58u) = *(volatile u16*)(world + 0x4020u);
  *(u16*)(scratch + 0x5au) = *(volatile u16*)(world + 0x4022u);
  func_8014D290();
}
