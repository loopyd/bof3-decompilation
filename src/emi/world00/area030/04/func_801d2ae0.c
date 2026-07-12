#include "internal.h"

/* @behavior either advances the local AREA030 scratch record from the shared world
 * state or falls back to the game-side helper when the countdown gate is not
 * active.
 * @source 0x801d2ae0 FUN_801d2ae0
 */
void func_801d2ae0(void) {
  volatile u8* scratch;
  volatile u8* world;

  world = (volatile u8*)0x80140000u;

  if ((*(volatile s16*)(world + 0x4006u) < 1) || (world[0x3fc9u] == 0u)) {
    func_80196070();
    return;
  }

  scratch = ((volatile u8**)0x1f800000u)[0x11];
  *(volatile u32*)(scratch + 0x34u) = *(volatile u32*)(world + 0x3ffcu);
  *(volatile u32*)(scratch + 0x38u) = *(volatile u32*)(world + 0x4000u);
  scratch[0x2au] = world[0x3ff2u];

  scratch = ((volatile u8**)0x1f800000u)[0x11];
  scratch[0x49u] = world[0x4011u];
  scratch = ((volatile u8**)0x1f800000u)[0x11];
  scratch[0x4au] = world[0x4012u];
  scratch = ((volatile u8**)0x1f800000u)[0x11];
  scratch[0x4bu] = world[0x4013u];
  scratch = ((volatile u8**)0x1f800000u)[0x11];
  *(volatile u32*)(scratch + 0x50u) = *(volatile u32*)(world + 0x4018u);
  *(volatile u32*)(scratch + 0x54u) = *(volatile u32*)(world + 0x401cu);
  *(volatile u16*)(scratch + 0x58u) = *(volatile u16*)(world + 0x4020u);
  *(volatile u16*)(scratch + 0x5au) = *(volatile u16*)(world + 0x4022u);
  func_8014d290();
}
