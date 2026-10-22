#include "bof3/battle/battle15_internal.h"

/* @behavior configures a battle action target, setting the action state for the
 * given battler. resolves actor index, target reference, and initialises
 * approach/motion flags depending on target defensive state.
 * @source 0x800AAEBC
 * @status partial
 * @match 79.53
 * @residual allocation: 101/127 instructions with an exact 508-byte image. The
 *   two flag reads must stay volatile casts: both arms then keep their own
 *   D_801EC3A4 store, and the two loads merge into one shared address instead of
 *   folding through `at`, which is why the flag byte lands in $a1. The remaining
 *   differences are the pervasive $a1/$a2 role swap (the original keeps
 *   action_slot in $a1 and target_flags in $a2) and the schedule around the
 *   joined sign-extension seed.
 */
void func_800AAEBC(s16 target_index, u8 battler_index) {
  u8  action_slot;
  u8  target_flags;
  u32 enemy;

  action_slot = (u8)func_801E590C(0u, 1u);

  D_801EC33B[action_slot * 0x78] = 1u;

  if (battler_index < 3u) {
    *(u32*)&D_801EC3A4[action_slot * 0x78] =
        (u32)&D_80145E90[battler_index];
    target_flags = ((volatile u8*)D_80145FB0)[battler_index * 0x140];
  } else {
    enemy = battler_index - 3u;
    *(u32*)&D_801EC3A4[action_slot * 0x78] =
        (u32)&D_801EB2E8[battler_index * 0x118];
    target_flags = ((volatile u8*)D_801EB72C)[enemy * 0x118];
  }

  if (target_index < 0) {
    *(s32*)&D_801EC390[action_slot * 0x78] = abs((s32)target_index);
    D_801EC357[action_slot * 0x78] = 1u;
  } else {
    *(s32*)&D_801EC390[action_slot * 0x78] = (s32)target_index;
    D_801EC357[action_slot * 0x78] = 2u;
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
