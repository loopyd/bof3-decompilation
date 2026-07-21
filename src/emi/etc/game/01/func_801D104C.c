#include "internal.h"

/* possible name: front_scene_pre_dispatch_gate
 * @behavior runs before the main GAME.EMI entry-1 state handler each frame and
 * redirects early start-button transitions into either state `2` or state `7`.
 * @source 0x801D104C
 */
void func_801D104C(void) {
  /* The original keeps &GAME_FRONT_INPUT_GATE in s0 and reaches
   * D_80143B40/GAME_FRONT_STATE/GAME_FRONT_TIMER as negative byte offsets
   * (lhu/sh -0xF3/-0x23/-0x13(s0)). EFFECT_BUSY, GAME_FRONT_PAD_STATE and
   * GAME_FRONT_POPUP_WORD are standalone globals (each its own lui). */
  register volatile u8* front_gate __asm__("s0") = &GAME_FRONT_INPUT_GATE;

  if (*front_gate != 0u) {
    if ((GAME_FRONT_PAD_STATE & GAME_FRONT_START_MASK) != 0u) {
      if (func_80162D00()) {
        if (GAME_FRONT_EFFECT_BUSY == 0u) {
          if (D_80143B40 == 0u) {
            if (GAME_FRONT_STATE < 3u) {
              GAME_FRONT_TIMER = 1u;
              GAME_FRONT_STATE = 2u;
            } else if ((GAME_FRONT_POPUP_WORD &
                        GAME_FRONT_POPUP_PENDING_MASK) ==
                       GAME_FRONT_POPUP_PENDING_OPEN) {
              func_8014ECAC(0);
              func_801D1134();
              game_queue_frontend_cue(0x105u);
              *front_gate = 0u;
              GAME_FRONT_STATE = 7u;
            }
          }
        }
      }
    }
  }
}
