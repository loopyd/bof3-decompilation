#include "internal.h"

/* @behavior initializes the primary battle selection-grid scratch band from the
 * current active message slot and the saved group/page/cursor bytes.
 * @source 0x80097EC0
 */
void func_80097EC0(void) {
  volatile u8* state;
  volatile u8* message_slot;
  volatile u8* active_selection_slot;
  u32          source_slot_offset;
  u32          active_selection_flags;

  state = BATTLE_UNK_80148570_BASE;
  state[0] = 1u;
  state[1] = 8u;
  message_slot = (volatile u8*)BATTLE_ACTIVE_MESSAGE_SLOT_PTR;
  state[2] = 2u;
  state[3] = 2u;

  source_slot_offset = (u32)message_slot[5] * 3u;
  state[0xa] = message_slot[5];
  state[0xb] = BATTLE_SELECTION_SAVED_GROUP(message_slot[5]);
  *(volatile u16*)(state + 0x10) =
      BATTLE_SELECTION_SAVED_SCROLL(message_slot[5]);
  state[0xd] = 0xffu;
  state[8] = 2u;
  state[9] = 0u;
  state[0xc] = BATTLE_SELECTION_SAVED_CURSOR(message_slot[5]);

  active_selection_slot = (volatile u8*)BATTLE_ACTIVE_SELECTION_SLOT_PTR;
  active_selection_flags = *(volatile u32*)(active_selection_slot + 0x10);
  *(volatile u16*)(state + 0x14) = (u16)(active_selection_flags & 2u);
  if ((active_selection_flags & 0x20002u) != 2u) {
    *(volatile u16*)(state + 0x14) = 0u;
  }

  *(volatile u16*)(state + 4) = 0x140u;
  *(volatile u16*)(state + 6) = 0x3fu;
  BATTLE_UNK_801485DC = 0u;
  BATTLE_UNK_801485DD = 8u;
  BATTLE_UNK_801485DE = 0u;
  BATTLE_UNK_80148656 = message_slot[5];
}
