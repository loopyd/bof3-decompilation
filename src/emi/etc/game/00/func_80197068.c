#include "internal.h"

struct GameSelState {
  u8           pad[0x3b90];
  volatile u16 state;
  volatile u16 substate;
};

#define GAME_SEL_STATE ((volatile struct GameSelState*)0x80140000u)

/* @behavior resets selection state and continuously dispatches the current
 * selection callback before advancing the front-end loop.
 * @source 0x80197068 FUN_80197068
 */
void func_80197068(void) {
  const GameEntry0StateHandler* callbacks;

  GAME_SEL_STATE->state = 0u;
  GAME_SEL_STATE->substate = 0u;
  func_8014ba04();
  func_80158e50();

  while (1) {
    callbacks = GAME_SELECTION_CALLBACK_TABLE;
    callbacks[GAME_SEL_STATE->state]();
    func_80198cac();
    func_8014b87c(1u);
  }
}
