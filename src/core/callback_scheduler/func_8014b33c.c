#include "internal.h"

extern volatile GameCallbackSlot* DAT_80143d40;
extern GameCallbackSlot           DAT_80143b40;

void func_8014b900(int slot_index);

/* @behavior walks the callback slot table and dispatches each non-empty slot by
 * index.
 * @source 0x8014b33c FUN_8014b33c
 */
void func_8014b33c(void) {
  GameCallbackSlot*          slot_end;
  volatile GameCallbackSlot* next_slot;
  s32                        slot_index;

  slot_index = 0;
  DAT_80143d40 = &DAT_80143b40;
  slot_end = &DAT_80143b40 + 4;

  do {
    if (DAT_80143d40->state != GAME_CALLBACK_SLOT_STATE_EMPTY) {
      func_8014b900(slot_index & 0xff);
    }

    next_slot = DAT_80143d40 + 1;
    DAT_80143d40 = next_slot;
  } while ((slot_index += 1, next_slot < slot_end));
}
