#include "internal.h"

/* This access needs the target-local symbol relocation, rather than the shared
 * PSX_PTR alias used by other frontend code. */
#undef D_8014832E

/* @behavior waits for the selection effect to close, resets frontend-local
 * phases and EXE flags, installs the next callback, then advances the state.
 * @source 0x801D0E54
 */
void game_front_finish_selection(void) {
  if (GAME_FRONT_EFFECT_BUSY == 0u) {
    GAME_FRONT_FADE_PHASE = 0u;
    GAME_FRONT_WINDOW_PHASE = 0u;
    GAME_FRONT_INPUT_GATE = 0u;
    game_front_stop_selection_fx();
    func_80161808(0u);
    func_8019611C();
    D_80144FC3 = 0u;
    D_80144FC2 = 0u;
    D_80144FC1 = 0u;
    D_80144FC0 = 0u;
    D_8014832E = 0u;
    /* MATCHING_AID: keep a0=16 in jal delay slot after flag clears */
    CLOBBER_A0();
    func_801A7704(16);
    func_8014B854(0, func_80197068);
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
