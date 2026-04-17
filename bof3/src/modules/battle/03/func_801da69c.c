#include "internal.h"

/* does: reports whether the current enemy slot can reuse a UI owner slot
 * without conflicting with another non-hidden slot carrying the same kind byte.
 * @source: 0x801da69c FUN_801da69c
 */
u8 func_801da69c(u32 arg0) {
  s8 owner_index;
  u8 slot_kind;

  arg0 &= 0xffu;
  slot_kind = BOF3_BATTLE_ENEMY_SLOT_KIND(arg0);
  owner_index = 7;
  while (owner_index >= 0) {
    if (BOF3_BATTLE_UI_BYTE_833A((u32)(owner_index + 5)) == (u8)(arg0 + 3u)) {
      break;
    }
    owner_index -= 1;
  }

  if (owner_index < 7) {
    s8 scan_index;

    scan_index = 7;
    while (scan_index > owner_index) {
      u8 ui_kind;
      u8 ui_mode;

      ui_mode = BOF3_BATTLE_UI_BYTE_8333_INDEX((u32)(scan_index + 5));
      if ((ui_mode != 2u) && (ui_mode != 3u)) {
        ui_kind = BOF3_BATTLE_UI_BYTE_833A((u32)(scan_index + 5));
        if (BOF3_BATTLE_ENEMY_SLOT_KIND(ui_kind - 3u) == slot_kind) {
          return 0u;
        }
      }
      scan_index -= 1;
    }
  }

  return 1u;
}
