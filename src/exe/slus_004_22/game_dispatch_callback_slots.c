#include "internal.h"

extern volatile GameCallbackSlot* gameCallbackSlotCursor; /* @kind: bss */
extern GameCallbackSlot           D_80143B40;

extern void game_close_callback_slot(int slot_index);

/* @behavior walks the callback slot table and dispatches each non-empty slot by
 * index.
 * @source 0x8014B33C
 */
void game_dispatch_callback_slots(void) {
  GameCallbackSlot*          slot_end;
  volatile GameCallbackSlot* next_slot;
  s32                        slot_index;

  slot_index = 0;
  gameCallbackSlotCursor = &D_80143B40;
  slot_end = &D_80143B40 + 4;

  do {
    if (gameCallbackSlotCursor->state != GAME_CALLBACK_SLOT_STATE_EMPTY) {
      game_close_callback_slot(slot_index & 0xff);
    }

    next_slot = gameCallbackSlotCursor + 1;
    gameCallbackSlotCursor = next_slot;
  } while ((slot_index += 1, next_slot < slot_end));
}
