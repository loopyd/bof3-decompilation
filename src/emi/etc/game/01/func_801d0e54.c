#include "internal.h"

/* @behavior waits for the selection effect to close, resets frontend-local
 * phases and EXE flags, installs the next callback, then advances the state.
 * @source 0x801D0E54
 */
void func_801D0E54(void) {
  if (GAME_FRONT_EFFECT_BUSY == 0u) {
    GAME_FRONT_FADE_PHASE = 0u;
    GAME_FRONT_WINDOW_PHASE = 0u;
    GAME_FRONT_INPUT_GATE = 0u;
    func_801D1184();
    game_set_frontend_layout_bank(0u);
    func_8019611C();
    D_80144FC3 = 0u;
    D_80144FC2 = 0u;
    D_80144FC1 = 0u;
    D_80144FC0 = 0u;
    D_8014832E = 0u;
    func_801A7704(16);
    func_8014B854(0, func_80197068);
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
