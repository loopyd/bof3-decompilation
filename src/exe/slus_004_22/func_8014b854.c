#include "internal.h"

/* possible name: game_install_callback_slot
 * @behavior stores one callback entrypoint in the requested slot and marks it ready
 * for thread open state `2`.
 * @source 0x8014b854 FUN_8014b854
 */
extern GameCallbackEntry D_80143B44;
extern u16               D_80143B40;

void func_8014b854(int slot_index, GameCallbackEntry callback) {
  int slot_offset = slot_index << 7;

  *(volatile GameCallbackEntry*)((u8*)&D_80143B44 + slot_offset) = callback;
  *(volatile u16*)((u8*)&D_80143B40 + slot_offset) =
      GAME_CALLBACK_SLOT_STATE_OPEN;
}
