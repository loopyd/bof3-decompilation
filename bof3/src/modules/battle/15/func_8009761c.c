#include "internal.h"

/* does: commits one battle selection-kind flag class into the active slot byte
 * and routes into the later result/finalize substate corridor.
 * @source: 0x8009761c FUN_8009761c
 */
void func_8009761c(void) {
  u8* selection_kind_table;
  u8  selected_kind;
  u8  selected_kind_flags;
  u8  active_selection_kind;

  selection_kind_table = battle_resolve_selection_kind_table(
      *(volatile u8*)(0x80150000u - 0x79aau),
      *(volatile u8*)(0x80150000u - 0x7a85u), 1u);
  selected_kind = selection_kind_table[*(volatile u8*)(0x80150000u - 0x7a84u)];
  selected_kind_flags =
      *(volatile u8*)((0x801d0000u - 0x58e8u) + ((u32)selected_kind * 0x14u));

  if ((selected_kind_flags & 0x40u) != 0u) {
    if ((selected_kind_flags & 0x10u) != 0u) {
      BOF3_BATTLE_SELECTION_ROOT_STATE = 5u;
    } else {
      BOF3_BATTLE_SELECTION_ROOT_STATE = 4u;
    }
    return;
  }

  if ((selected_kind_flags & 0x80u) != 0u) {
    BOF3_BATTLE_ACTIVE_SELECTION_SLOT_PTR[0] = 0xc0u;
  } else if ((selected_kind_flags & 0x10u) != 0u) {
    if ((selected_kind_flags & 0x20u) != 0u) {
      BOF3_BATTLE_ACTIVE_SELECTION_SLOT_PTR[0] = 0x40u;
    } else {
      active_selection_kind = 0x80u;
      BOF3_BATTLE_ACTIVE_SELECTION_SLOT_PTR[0] = active_selection_kind;
    }
  } else {
    active_selection_kind =
        ((volatile u8*)BOF3_BATTLE_ACTIVE_MESSAGE_SLOT_PTR)[5];
    BOF3_BATTLE_ACTIVE_SELECTION_SLOT_PTR[0] = active_selection_kind;
  }

  BOF3_BATTLE_SELECTION_ROOT_STATE = 4u;
  BOF3_BATTLE_SELECTION_SUBSTATE = 3u;
}
