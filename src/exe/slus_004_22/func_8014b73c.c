#include "internal.h"

extern volatile GameCallbackSlot* D_80143D40;
extern GameCallbackSlot           D_80143B40;

/* possible name: game_slot_scheduler_tick
 * @behavior walks the EXE callback slot table, opens ready threads, decrements
 * yield timers, and switches into any thread whose slot reached dispatch state.
 * @source 0x8014B73C
 */
void func_8014B73C(void) {
  u16                        new_var;
  GameCallbackSlot*          current_slot;
  volatile GameCallbackSlot* next_slot;
  u16                        state;
  unsigned short             countdown;

  state = 0x80143d40u;
  current_slot = (D_80143D40 = &D_80143B40);

  do {
    state = D_80143D40->state;

    switch (state) {
      case GAME_CALLBACK_SLOT_STATE_OPEN:
        EnterCriticalSection();
        (current_slot = D_80143D40)->thread_id =
            OpenTh((long (*)())D_80143D40->callback, D_80143D40->open_arg,
                   D_80143D40->open_arg_2);
        ExitCriticalSection();
        goto dispatch;

      case GAME_CALLBACK_SLOT_STATE_YIELD:
        new_var = D_80143D40->countdown;
        countdown = (u16)(new_var - 1u);
        D_80143D40->countdown = countdown;
        if (countdown != 0u) {
          break;
        }
        goto dispatch;

      case GAME_CALLBACK_SLOT_STATE_SWITCH:
      dispatch:
        D_80143D40->state = (state = GAME_CALLBACK_SLOT_STATE_IDLE);
        countdown = D_80143D40->thread_id;
        ChangeTh(countdown);
        break;

      default:
        break;
    }

    next_slot = D_80143D40 + 1;
    D_80143D40 = next_slot;
  } while (next_slot < ((u32)((volatile GameCallbackSlot*)state)));
}
