#include "internal.h"

/* does: selects layout bank `2`, requests `DEMO.EMI`, waits for the loader,
 * seeds cue `0x8d`, then arms the local fade/window state.
 * @source: 0x801d0c90 FUN_801d0c90
 * @source: docs/specs/runtime/game-overlay.md
 */
void func_801d0c90(void) {
  u16 state;

  game_set_frontend_layout_bank(2u);
  emi_stream_init_slot(0x25fu);

  while (!func_80162d00()) {
    func_8014b87c(1u);
  }

  game_stage_shared_palette_bank();
  BOF3_GAME_FRONT_PALETTE_STAGE_SERIAL += 1u;
  game_set_active_selection_cue(0x8du, 100, 8);

  BOF3_GAME_FRONT_FADE_PHASE = 1u;
  state = BOF3_GAME_FRONT_STATE;
  BOF3_GAME_FRONT_BANNER_SCROLL = 200u;
  BOF3_GAME_FRONT_BANNER_ALPHA = 0u;
  BOF3_GAME_FRONT_WINDOW_PHASE = 0u;
  BOF3_GAME_FRONT_WINDOW_ALPHA_PRIMARY = 0u;
  BOF3_GAME_FRONT_WINDOW_ALPHA_SECONDARY = 0u;
  BOF3_GAME_FRONT_INPUT_GATE = 1u;
  BOF3_GAME_FRONT_STATE = state + 1u;
}
