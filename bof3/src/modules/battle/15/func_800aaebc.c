#include "internal.h"

/* does: configures a battle action target, setting the action state for the
 * given battler. resolves actor index, target reference, and initialises
 * approach/motion flags depending on target defensive state.
 * @source: 0x800aaebc FUN_800aaebc
 */
void func_800aaebc(s16 target_index, u8 battler_index) {
  volatile u8*  battler_data;
  u8            action_slot;
  u8            target_flags;
  u8            enemy;

  action_slot = (u8)((s32(*)(s32, s32))0x801e590cu)(0, 1);

  REG8(0x801ec33bu + ((u32)action_slot * 0x78u)) = 1u;

  if (battler_index < 3u) {
    battler_data = (volatile u8*)(0x80145e90u + ((u32)battler_index * 0x140u));
    target_flags = REG8(0x80145fb0u + ((u32)battler_index * 0x140u));
  } else {
    enemy = battler_index - 3u;
    battler_data = (volatile u8*)(0x801eb2e8u + ((u32)enemy * 0x118u));
    target_flags = REG8(0x801eb72cu + ((u32)enemy * 0x118u));
  }

  if (target_index < 0) {
    target_index = -target_index;
    REG32(0x801ec390u + ((u32)action_slot * 0x78u)) = (s32)target_index;
    REG8(0x801ec357u + ((u32)action_slot * 0x78u)) = 1u;
  } else {
    REG32(0x801ec390u + ((u32)action_slot * 0x78u)) = (s32)target_index;
    REG8(0x801ec357u + ((u32)action_slot * 0x78u)) = 2u;
  }

  if (!(target_flags & 0x2u)) {
    REG8(0x801ec337u + ((u32)action_slot * 0x78u)) = 4u;
    return;
  }

  if (!(target_flags & 0x20u) && (target_flags & 0x8u)) {
    REG8(0x801ec357u + ((u32)action_slot * 0x78u)) = 1u;
    REG8(0x801ec337u + ((u32)action_slot * 0x78u)) = 2u;
    return;
  }

  REG8(0x801ec337u + ((u32)action_slot * 0x78u)) = 0u;
}
