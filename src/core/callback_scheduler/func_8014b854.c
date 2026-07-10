#include "internal.h"

/* possible name: game_install_callback_slot
 * does: stores one callback entrypoint in the requested slot and marks it ready
 * for thread open state `2`.
 * @source: 0x8014b854 FUN_8014b854
 * @source: docs/specs/runtime/game-overlay.md
 * @source: processed/inventory/inventory.sqlite (function metadata and refs)
 */
extern Bof3CallbackEntry DAT_80143b44;
extern u16               DAT_80143b40;

void func_8014b854(int slot_index, Bof3CallbackEntry callback) {
  int slot_offset = slot_index << 7;

  *(volatile Bof3CallbackEntry*)((u8*)&DAT_80143b44 + slot_offset) = callback;
  *(volatile u16*)((u8*)&DAT_80143b40 + slot_offset) =
      GAME_CALLBACK_SLOT_STATE_OPEN;
}
