#include "bof3/battle/battle03_internal.h"

extern int rand(void);
/* @behavior resolves one battler-kind-derived result code, updates the global kind
 * selection, and returns one mode byte based on the chosen kind flags.
 * @source 0x801D3844
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 resolveKindResultMode(void) {
  u16* state;
  u8   kind;
  u8   kind_flags;
  u16  selected_kind;

  /* NOTE: the original keeps this RAM address in s0 across the selector path. */
  state = (u16*)&BATTLE_GLOBAL_HALF_63C0;

  if (*state == 36) {
    kind = BATTLE_RANDOM_TABLE_AC58_DATA[rand() & 0x1fu];
  } else {
    kind = 0u;
  }

  if ((*state == 37) || (*state == 140)) {
    kind = BATTLE_RANDOM_TABLE_AC78_DATA[rand() & 0x1fu];
  }

  kind_flags = ABILITY_OBJECTS[kind].targeting_flags;
  selected_kind = kind;
  BATTLE_GLOBAL_HALF_63C0 = selected_kind;
  *(volatile u16*)(BATTLE_GLOBAL_PTR_6380 + 2) = selected_kind;

  if ((kind_flags & 0x10u) != 0u) {
    if ((kind_flags & 0x80u) != 0u) {
      if ((kind_flags & 0x40u) == 0u) {
        return 0xc0u;
      }
    }

    if (BATTLE_GLOBAL_BYTE_6374 < 3u) {
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
    return BATTLE_GLOBAL_BYTE_6374;
  }

  if (BATTLE_GLOBAL_BYTE_6374 < 3u) {
    if ((kind_flags & 0x20u) != 0u) {
      return func_800A955C();
    }
  } else if ((kind_flags & 0x20u) == 0u) {
    return func_800A955C();
  }

  return func_800A94A8();
}
