#include "internal.h"

/* @behavior waits for the selection effect to close, resets frontend-local
 * phases and EXE flags, installs the next callback, then advances the state.
 * @source 0x801d0e54 FUN_801d0e54
 */
void func_801d0e54(void) {
  if (GAME_FRONT_EFFECT_BUSY == 0u) {
    GAME_FRONT_FADE_PHASE = 0u;
    GAME_FRONT_WINDOW_PHASE = 0u;
    GAME_FRONT_INPUT_GATE = 0u;
    func_801d1184();
    game_set_frontend_layout_bank(0u);
    func_8019611c();
    DAT_80144fc3 = 0u;
    DAT_80144fc2 = 0u;
    DAT_80144fc1 = 0u;
    DAT_80144fc0 = 0u;
    DAT_8014832e = 0u;
    func_801a7704(16);
    func_8014b854(0, func_80197068);
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
