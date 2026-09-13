#include "bof3/battle/battle15_internal.h"

/* @behavior configures a battle action target, setting the action state for the
 * given battler. resolves actor index, target reference, and initialises
 * approach/motion flags depending on target defensive state.
 * @source 0x800AAEBC
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void func_800AAEBC(s16 target_index, u8 battler_index) {
  u8  action_slot;
  u8  *slot_entry;
  u32 target_flags;
  u32 enemy;
  s32 magnitude;

  action_slot = (u8)func_801E590C(0u, 1u);

  D_801EC33B[action_slot * 0x78] = 1u;

  if (battler_index < 3u) {
    *(u32*)&D_801EC3A4[action_slot * 0x78] =
        (u32)&D_80145E90[battler_index];
    target_flags = ((u8*)D_80145FB0)[battler_index * 0x140];
  } else {
    enemy = battler_index - 3u;
    *(u32*)&D_801EC3A4[action_slot * 0x78] =
        (u32)&D_801EB2E8[battler_index * 0x118];
    target_flags = ((u8*)D_801EB72C)[enemy * 0x118];
  }

  if (target_index < 0) {
    *(s32*)&D_801EC390[action_slot * 0x78] =
        (magnitude = abs((s32)target_index));
    D_801EC357[action_slot * 0x78] = 1u;
  } else {
    /* The do-while(0) wrapper and slot_entry temporary came from a prior
     * permuter experiment to preserve the positive-arm store schedule.
     * Its earlier exact result included now-removed barriers and is not
     * evidence of a current clean-C match. */
    do {
      slot_entry = &D_801EC390[action_slot * 0x78];
      *(s32*)slot_entry = (s32)target_index;
      D_801EC357[action_slot * 0x78] = 2u;
    } while (0);
  }

  if (!(target_flags & 0x2u)) {
    D_801EC337[action_slot * 0x78] = 4u;
    return;
  }

  if (!(target_flags & 0x20u)) {
    if (target_flags & 0x8u) {
      D_801EC357[action_slot * 0x78] = 1u;
      D_801EC337[action_slot * 0x78] = 2u;
      return;
    }
  }

  D_801EC337[action_slot * 0x78] = 0u;
}
