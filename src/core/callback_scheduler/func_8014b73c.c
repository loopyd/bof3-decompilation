#include "internal.h"

extern volatile GameCallbackSlot* DAT_80143d40;
extern GameCallbackSlot           DAT_80143b40;

/* possible name: game_slot_scheduler_tick
 * does: walks the EXE callback slot table, opens ready threads, decrements
 * yield timers, and switches into any thread whose slot reached dispatch state.
 * @source: 0x8014b73c FUN_8014b73c
 * @source: docs/specs/runtime/game-overlay.md
 * @source: docs/specs/runtime/boot-sequence.md
 */
void func_8014b73c(void) {
  u16                        new_var;
  GameCallbackSlot*          current_slot;
  volatile GameCallbackSlot* next_slot;
  u16                        state;
  unsigned short             countdown;

  state = 0x80143d40u;
  current_slot = (DAT_80143d40 = &DAT_80143b40);

  do {
    state = DAT_80143d40->state;

    switch (state) {
      case GAME_CALLBACK_SLOT_STATE_OPEN:
        func_8017ee0c();
        (current_slot = DAT_80143d40)->thread_id =
            OpenTh((long (*)())DAT_80143d40->callback, DAT_80143d40->open_arg,
                   DAT_80143d40->open_arg_2);
        func_8017ee1c();
        goto dispatch;

      case GAME_CALLBACK_SLOT_STATE_YIELD:
        new_var = DAT_80143d40->countdown;
        countdown = (u16)(new_var - 1u);
        DAT_80143d40->countdown = countdown;
        if (countdown != 0u) {
          break;
        }
        goto dispatch;

      case GAME_CALLBACK_SLOT_STATE_SWITCH:
      dispatch:
        DAT_80143d40->state = (state = GAME_CALLBACK_SLOT_STATE_IDLE);
        countdown = DAT_80143d40->thread_id;
        ChangeTh(countdown);
        break;

      default:
        break;
    }

    next_slot = DAT_80143d40 + 1;
    DAT_80143d40 = next_slot;
  } while (next_slot < ((u32)((volatile GameCallbackSlot*)state)));
}
