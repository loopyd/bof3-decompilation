#include "internal.h"

extern volatile GameCallbackSlot* D_80143D40;
extern GameCallbackSlot           D_80143B40;

extern void func_8014B900(int slot_index);

/* @behavior walks the callback slot table and dispatches each non-empty slot by
 * index.
 * @source 0x8014B33C
 */
void func_8014B33C(void) {
  GameCallbackSlot*          slot_end;
  volatile GameCallbackSlot* next_slot;
  s32                        slot_index;

  slot_index = 0;
  D_80143D40 = &D_80143B40;
  slot_end = &D_80143B40 + 4;

  do {
    if (D_80143D40->state != GAME_CALLBACK_SLOT_STATE_EMPTY) {
      func_8014B900(slot_index & 0xff);
    }

    next_slot = D_80143D40 + 1;
    D_80143D40 = next_slot;
  } while ((slot_index += 1, next_slot < slot_end));
}
