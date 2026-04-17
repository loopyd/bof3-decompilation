#include "internal.h"

/* does: resolves one battler-kind-derived result code, updates the global kind
 * selection, and returns one mode byte based on the chosen kind flags.
 * @source: 0x801d3844 FUN_801d3844
 */
u8 func_801d3844(void) {
  u8  kind;
  u8  kind_flags;
  u16 selected_kind;

  kind = 0u;
  if (BOF3_BATTLE_GLOBAL_HALF_63C0 == 0x24u) {
    kind = BOF3_BATTLE_RANDOM_TABLE_AC58[func_8017e3d4() & 0x1fu];
  }
  if ((BOF3_BATTLE_GLOBAL_HALF_63C0 == 0x25u) ||
      (BOF3_BATTLE_GLOBAL_HALF_63C0 == 0x8cu)) {
    kind = BOF3_BATTLE_RANDOM_TABLE_AC78[func_8017e3d4() & 0x1fu];
  }

  kind_flags = *(volatile u8*)(0x801ca718u + ((u32)kind * 0x14u));
  selected_kind = kind;
  BOF3_BATTLE_GLOBAL_HALF_63C0 = selected_kind;
  *(volatile u16*)(BOF3_BATTLE_GLOBAL_PTR_6380 + 2) = selected_kind;

  if ((kind_flags & 0x10u) != 0u) {
    if (((kind_flags & 0x80u) != 0u) && ((kind_flags & 0x40u) == 0u)) {
      return 0xc0u;
    }

    if (BOF3_BATTLE_GLOBAL_BYTE_6374 < 3u) {
      if ((kind_flags & 0x20u) != 0u) {
        return 0x40u;
      }
      return 0x80u;
    }

    if ((kind_flags & 0x20u) != 0u) {
      return 0x80u;
    }
    return 0x40u;
  }

  if ((kind_flags & 0x40u) == 0u) {
    return BOF3_BATTLE_GLOBAL_BYTE_6374;
  }

  if (BOF3_BATTLE_GLOBAL_BYTE_6374 < 3u) {
    if ((kind_flags & 0x20u) != 0u) {
      return func_800a955c();
    }
  } else if ((kind_flags & 0x20u) == 0u) {
    return func_800a955c();
  }

  return func_800a94a8();
}
