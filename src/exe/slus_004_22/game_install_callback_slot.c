#include "internal.h"

/* @behavior stores one callback entrypoint in the requested slot and marks it ready
 * for thread open state `2`.
 * @source 0x8014B854
 */
extern GameCallbackEntry D_80143B44;
extern u16               D_80143B40;

void game_install_callback_slot(int slot_index, GameCallbackEntry callback) {
  int slot_offset = slot_index << 7;

  *(volatile GameCallbackEntry*)((u8*)&D_80143B44 + slot_offset) = callback;
  *(volatile u16*)((u8*)&D_80143B40 + slot_offset) =
      GAME_CALLBACK_SLOT_STATE_OPEN;
}
