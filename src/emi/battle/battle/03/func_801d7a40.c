#include "internal.h"

/* @behavior draws one five-slot icon strip plus the two optional trailing icons,
 * choosing either a flat icon draw or the tile helper based on the current size
 * bytes.
 * @source 0x801d7a40 FUN_801d7a40
 */
void func_801d7a40(s16 arg0, s16 arg1) {
  u8  index;
  u32 color;
  u8  size;

  color = 0u;
  index = 0u;
  do {
    if (index != BATTLE_GLOBAL_BYTE_62F4) {
      color = 0x80u;
      if ((index >= 2u) && (index < 4u) &&
          ((*(volatile u32*)(BATTLE_GLOBAL_PTR_BF08 + 0x128u) & 2u) != 0u)) {
        color = 0x40u;
      }
      size = BATTLE_GLOBAL_BYTE_62FC(index);
      func_801644d8(index,
                    (s32)arg0 + BATTLE_ICON_OFFSET_TABLE_AE94[index * 2u] -
                        (s32)(size >> 1),
                    (s32)arg1 + BATTLE_ICON_OFFSET_TABLE_AE94[index * 2u + 1] -
                        (s32)(size >> 1),
                    (s32)size + 0x10, (s32)size + 0x10, color);
    }
    index += 1u;
  } while (index < 5u);

  color = 0x80u;
  if ((BATTLE_GLOBAL_BYTE_62F4 < 5u) && (BATTLE_GLOBAL_BYTE_62F4 >= 2u) &&
      ((*(volatile u32*)(BATTLE_GLOBAL_PTR_BF08 + 0x128u) & 2u) != 0u)) {
    color = 0x40u;
  }

  size = BATTLE_GLOBAL_BYTE_62FC(BATTLE_GLOBAL_BYTE_62F4);
  if (size == 0u) {
    func_801644d8(
        BATTLE_GLOBAL_BYTE_62F4,
        (s32)arg0 + BATTLE_ICON_OFFSET_TABLE_AE94[BATTLE_GLOBAL_BYTE_62F4 * 2u],
        (s32)arg1 +
            BATTLE_ICON_OFFSET_TABLE_AE94[BATTLE_GLOBAL_BYTE_62F4 * 2u + 1],
        0x10, 0x10, color);
  } else {
    func_801d7d10(
        BATTLE_GLOBAL_BYTE_62F4,
        arg0 + BATTLE_ICON_OFFSET_TABLE_AE94[BATTLE_GLOBAL_BYTE_62F4 * 2u] -
            (size >> 1),
        arg1 + BATTLE_ICON_OFFSET_TABLE_AE94[BATTLE_GLOBAL_BYTE_62F4 * 2u + 1] -
            (size >> 1),
        size + 0x10u, size + 0x10u, (u8)color);
  }

  if (BATTLE_GLOBAL_BYTE_6301 != 0u) {
    func_801d7d10(5u,
                  arg0 + BATTLE_ICON_OFFSET_TABLE_AE94[10] -
                      (BATTLE_GLOBAL_BYTE_6301 >> 1),
                  arg1 + BATTLE_ICON_OFFSET_TABLE_AE94[11] -
                      (BATTLE_GLOBAL_BYTE_6301 >> 1),
                  BATTLE_GLOBAL_BYTE_6301 + 0x10u,
                  BATTLE_GLOBAL_BYTE_6301 + 0x10u, 0x80u);
  }
  if (BATTLE_GLOBAL_BYTE_6302 != 0u) {
    func_801d7d10(6u,
                  arg0 + BATTLE_ICON_OFFSET_TABLE_AE94[12] -
                      (BATTLE_GLOBAL_BYTE_6302 >> 1),
                  arg1 + BATTLE_ICON_OFFSET_TABLE_AE94[13] -
                      (BATTLE_GLOBAL_BYTE_6302 >> 1),
                  BATTLE_GLOBAL_BYTE_6302 + 0x10u,
                  BATTLE_GLOBAL_BYTE_6302 + 0x10u, 0x80u);
  }
}
