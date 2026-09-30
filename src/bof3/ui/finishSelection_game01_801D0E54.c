#include "bof3/ui/game01_internal.h"

/* This access needs the target-local symbol relocation, rather than the shared
 * PSX_PTR alias used by other frontend code. */
#undef D_8014832E

/* @behavior waits for the selection effect to close, resets frontend-local
 * phases and EXE flags, installs the next callback, then advances the state.
 * @source 0x801D0E54
 * @status exact
 * @match 100.00
 * @residual none
 */
void finishSelection(void) {
  if (GAME_FRONT_EFFECT_BUSY == 0u) {
    /* Clear the fade/window/input phase bytes through non-volatile views: with
     * the plain volatile stores cc1 hoists `li a0,16` above them and leaves a
     * `nop` in the func_801A7704 jal delay slot. */
    *(u8*)&GAME_FRONT_FADE_PHASE = 0u;
    *(u8*)&GAME_FRONT_WINDOW_PHASE = 0u;
    *(u8*)&GAME_FRONT_INPUT_GATE = 0u;
    stopSelectionFx();
    func_80161808(0u);
    func_8019611C();
    *(u8*)&D_80144FC3 = 0u;
    *(u8*)&D_80144FC2 = 0u;
    *(u8*)&D_80144FC1 = 0u;
    *(u8*)&D_80144FC0 = 0u;
    *(u8*)&D_8014832E = 0u;

    func_801A7704(16);
    func_8014B854(0, func_80197068);
    GAME_FRONT_STATE = GAME_FRONT_STATE + 1u;
  }
}
