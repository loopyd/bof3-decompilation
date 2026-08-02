#include "internal.h"

/* @behavior selects layout bank `2`, requests `DEMO.EMI`, waits for the loader,
 * seeds cue `0x8d`, then arms the local fade/window state.
 * @source 0x801D0C90
 */
void func_801D0C90(void) {
  u16 state;

  func_80161808(2u);
  emi_stream_init_slot(0x25fu);

  while (!func_80162D00()) {
    func_8014B87C(1u);
  }

  game_stage_shared_palette_bank();
  /* MATCHING_AID: serial +=1 folded into a2 via comma so the store schedules
   * before the call and a2=8 lands in the jal delay slot; serial is
   * non-volatile here (only user in this overlay). */
  func_80161C20(0x8du, 100u,
                                (GAME_FRONT_PALETTE_STAGE_SERIAL += 1u, 8u));

  state = GAME_FRONT_STATE;
  GAME_FRONT_FADE_PHASE = 1u;
  GAME_FRONT_BANNER_SCROLL = 200u;
  GAME_FRONT_BANNER_ALPHA = 0u;
  GAME_FRONT_WINDOW_PHASE = 0u;
  GAME_FRONT_WINDOW_ALPHA_PRIMARY = 0u;
  GAME_FRONT_WINDOW_ALPHA_SECONDARY = 0u;
  GAME_FRONT_INPUT_GATE = 1u;
  GAME_FRONT_STATE = state + 1u;
}
