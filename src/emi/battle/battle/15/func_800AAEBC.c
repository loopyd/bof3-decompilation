#include "internal.h"

/* @behavior configures a battle action target, setting the action state for the
 * given battler. resolves actor index, target reference, and initialises
 * approach/motion flags depending on target defensive state.
 * @source 0x800AAEBC
 */
void func_800AAEBC(s16 target_index, u8 battler_index) {
  volatile u8* battler_data;
  u8           action_slot;
  u8           target_flags;
  u8           enemy;

  action_slot = (u8)FUNCTION_AT(s32 (*)(s32, s32), 0x801e590cu)(0, 1);

  PSX_REF(volatile u8, 0x801ec33bu + ((u32)action_slot * 0x78u)) = 1u;

  if (battler_index < 3u) {
    battler_data = (volatile u8*)((u32)BATTLE_PLAYER_BATTLER_BASE +
                                  ((u32)battler_index * 0x140u));
    target_flags = PSX_REF(volatile u8, 0x80145fb0u + ((u32)battler_index * 0x140u));
  } else {
    enemy = battler_index - 3u;
    battler_data =
        (volatile u8*)((u32)BATTLE_ENEMY_BATTLER_BASE + ((u32)enemy * 0x118u));
    target_flags = PSX_REF(volatile u8, 0x801eb72cu + ((u32)enemy * 0x118u));
  }

  if (target_index < 0) {
    target_index = -target_index;
    PSX_REF(volatile u32, 0x801ec390u + ((u32)action_slot * 0x78u)) = (s32)target_index;
    PSX_REF(volatile u8, 0x801ec357u + ((u32)action_slot * 0x78u)) = 1u;
  } else {
    PSX_REF(volatile u32, 0x801ec390u + ((u32)action_slot * 0x78u)) = (s32)target_index;
    PSX_REF(volatile u8, 0x801ec357u + ((u32)action_slot * 0x78u)) = 2u;
  }

  if (!(target_flags & 0x2u)) {
    PSX_REF(volatile u8, 0x801ec337u + ((u32)action_slot * 0x78u)) = 4u;
    return;
  }

  if (!(target_flags & 0x20u) && (target_flags & 0x8u)) {
    PSX_REF(volatile u8, 0x801ec357u + ((u32)action_slot * 0x78u)) = 1u;
    PSX_REF(volatile u8, 0x801ec337u + ((u32)action_slot * 0x78u)) = 2u;
    return;
  }

  PSX_REF(volatile u8, 0x801ec337u + ((u32)action_slot * 0x78u)) = 0u;
}
