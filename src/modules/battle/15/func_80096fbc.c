#include "internal.h"

/* @behavior arms the active selection slot for grid input, initializes the local
 * grid state, and advances from the confirm branch back into the root
 * selection update.
 * @source 0x80096fbc FUN_80096fbc
 */
void func_80096fbc(void) {
  u32 active_selection_flags;
  u8  selection_root_state;
  u8  source_slot;

  {
    u8* active_selection_slot;

    active_selection_slot = (u8*)BATTLE_ACTIVE_SELECTION_SLOT_PTR;
    active_selection_slot[1] = 4u;
  }
  func_80097ec0();

  active_selection_flags =
      *(u32*)(((u8*)BATTLE_ACTIVE_SELECTION_SLOT_PTR) + 0x10);
  if ((active_selection_flags & 2u) != 0u) {
    if ((active_selection_flags & 0x20000u) == 0u) {
      ((u8*)0x80150000u)[-0x7a85] = 2u;
    }
  }

  selection_root_state = BATTLE_SELECTION_ROOT_STATE;
  source_slot = ((u8*)BATTLE_ACTIVE_MESSAGE_SLOT_PTR)[5];
  ((u8*)0x80140000u)[0x62e4u] = 0u;
  BATTLE_SELECTION_ROOT_STATE = selection_root_state + 1u;
  ((u8*)0x80150000u)[-0x79aa] = source_slot;
}
