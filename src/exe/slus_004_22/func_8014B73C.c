#include "internal.h"

extern GameCallbackSlot* volatile gameCallbackSlotCursor; /* @kind: bss */
extern GameCallbackSlot           D_80143B40[4];

/* possible name: game_slot_scheduler_tick
 * @behavior walks the EXE callback slot table, opens ready threads, decrements
 * yield timers, and switches into any thread whose slot reached dispatch state.
 * @source 0x8014B73C
 */
void func_8014B73C(void) {
  GameCallbackSlot* top_slot;
  GameCallbackSlot* open_slot;
  GameCallbackSlot* dispatch_slot;
  GameCallbackSlot* next_slot;
  s32               state;
  u16               idle;

  gameCallbackSlotCursor = D_80143B40;
  idle = GAME_CALLBACK_SLOT_STATE_IDLE;

  do {
    top_slot = gameCallbackSlotCursor;
    state = top_slot->state;

    switch (state) {
      case GAME_CALLBACK_SLOT_STATE_OPEN:
        EnterCriticalSection();
        open_slot = gameCallbackSlotCursor;
        gameCallbackSlotCursor->thread_id =
            OpenTh((long (*)())open_slot->callback, open_slot->open_arg,
                   open_slot->open_arg_2);
        ExitCriticalSection();
        goto dispatch;

      case GAME_CALLBACK_SLOT_STATE_YIELD:
        if ((u16)--top_slot->countdown != 0u) {
          break;
        }
        goto dispatch;

      case GAME_CALLBACK_SLOT_STATE_SWITCH:
      dispatch:
        dispatch_slot = gameCallbackSlotCursor;
        dispatch_slot->state = idle;
        ChangeTh(dispatch_slot->thread_id);
        break;

      default:
        break;
    }

    next_slot = gameCallbackSlotCursor + 1;
    gameCallbackSlotCursor = next_slot;
  } while (next_slot < D_80143B40 + 4);
}
