#include "internal.h"

/* does: draws one full battler status strip for the current local battler
 * count, including frame pieces, bars, numeric readouts, and the surrounding
 * border.
 * @source: 0x801d750c FUN_801d750c
 */
void func_801d750c(s32 arg0, s32 arg1) {
  u8  index;
  u16 color;
  u8  red;
  u8  green;
  u8  blue;

  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0, func_8017a620(0, 0, 0x3c0, 0),
                0);
  func_8014e5a0(1u, 0x0cu);

  func_801d9ab4((s16)arg0, (s16)arg1, (BATTLE_GLOBAL_BYTE_62F0 * 2u) + 10u, 1);
  func_801d9ab4((s16)arg0, (s16)(arg1 + 0x18),
                (BATTLE_GLOBAL_BYTE_62F0 * 2u) + 11u, 1);
  func_801d9c80((s16)arg0, (s16)(arg1 + 2), BATTLE_GLOBAL_BYTE_62F0 + 0x0du, 1);

  color = *(volatile u16*)(0x80033a08u +
                           (((s32)BATTLE_GLOBAL_BYTE_4952 << 6) | 0x20));
  red = (color & 0x1fu) << 3;
  green = (color >> 2) & 0xf8u;
  blue = (color >> 7) & 0xf8u;

  index = 0u;
  if (BATTLE_GLOBAL_BYTE_62F0 != 0u) {
    do {
      s32                         slot_x;
      volatile Battle03LocalWork* battle_work;

      battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
      slot_x = arg0 + 0x14;
      func_801d9dbc((s16)slot_x, (s16)(arg1 + 0x0e), 9, 0x1e0u, 0u);
      func_801d9e9c((s16)slot_x, (s16)(arg1 + 0x0e),
                    BATTLE_ICON_OFFSET_TABLE_AE94[(index + 0x0d) * 2u],
                    BATTLE_ICON_OFFSET_TABLE_AE94[(index + 0x0d) * 2u + 2u], 0);

      if (BATTLE_LOCAL_HALF_8A(battle_work) != 0u) {
        func_801da5a8(
            (s16)slot_x, (s16)(arg1 + 0x0e),
            (s16)(arg0 + BATTLE_ICON_OFFSET_TABLE_AE94[(index + 0x0d) * 2u] +
                  0x14),
            (s16)(arg1 + 0x0e), 100u, 0u, 200u);
        if (BATTLE_ICON_OFFSET_TABLE_AE94[(index + 0x0d) * 2u + 2u] != 0u) {
          func_801da5a8(
              (s16)(arg0 + BATTLE_ICON_OFFSET_TABLE_AE94[(index + 0x0d) * 2u] +
                    0x14),
              (s16)(arg1 + 0x0e),
              (s16)(arg0 + BATTLE_ICON_OFFSET_TABLE_AE94[(index + 0x0d) * 2u] +
                    BATTLE_ICON_OFFSET_TABLE_AE94[(index + 0x0d) * 2u + 2u] +
                    0x14),
              (s16)(arg1 + 0x0e), 200u, 0u, 0u);
        }
      }

      func_801d94d4((s16)(arg0 + 6), (u16)(arg1 + 5),
                    ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4000u) != 0u)
                        ? 2u
                        : ((BATTLE_LOCAL_FLAGS_80(battle_work) >> 11) & 4u),
                    BATTLE_LOCAL_HALF_88(battle_work));

      func_801da078((s16)(arg0 + 0x3a), (s16)(arg1 + 6), 1);
      func_801da078((s16)(arg0 + 4), (s16)(arg1 + 0x0e), 0);

      func_801d94d4((s16)(arg0 + 0x48), (u16)(arg1 + 0x0f),
                    (BATTLE_LOCAL_HALF_8A(battle_work) != 0u &&
                     ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4000u) == 0u))
                        ? ((BATTLE_LOCAL_HALF_8A(battle_work) <
                            (BATTLE_LOCAL_HALF_92(battle_work) >> 2))
                           << 2)
                        : 2u,
                    BATTLE_LOCAL_HALF_88(battle_work));

      func_801d94d4((s16)(arg0 + 0x48), (u16)(arg1 + 8), 2u,
                    BATTLE_LOCAL_HALF_8A(battle_work));

      func_801da5a8((s16)(arg0 + 2), (s16)(arg1 + 2), (s16)(arg0 + 0x5d),
                    (s16)(arg1 + 2), red, green, blue);
      func_801da5a8((s16)(arg0 + 2), (s16)(arg1 + 3), (s16)(arg0 + 2),
                    (s16)(arg1 + 0x17), red, green, blue);
      func_801da4b4((s16)(arg0 + 0x5d), (s16)(arg1 + 2), (s16)(arg0 + 0x5d),
                    (s16)(arg1 + 0x17), red, green, blue);
      func_801da4b4((s16)(arg0 + 2), (s16)(arg1 + 0x17), (s16)(arg0 + 0x5d),
                    (s16)(arg1 + 0x17), red, green, blue);
      arg0 += 0x5e;
      index += 1u;
    } while (index < BATTLE_GLOBAL_BYTE_62F0);
  }
}
