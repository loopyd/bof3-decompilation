#include "internal.h"

/* possible name: front_scene_pre_dispatch_gate
 * @behavior runs before the main GAME.EMI entry-1 state handler each frame and
 * redirects early start-button transitions into either state `2` or state `7`.
 * @source 0x801D104C
 */
void func_801D104C(void) {
  /* The original keeps &GAME_FRONT_INPUT_GATE in s0 and reaches the nearby
   * front-state fields as negative byte offsets off that single base, not as
   * standalone globals:
   *   D_80143B40       (0x80143B40) = lhu -0xF3(s0)
   *   GAME_FRONT_STATE (0x80143C10) = lhu/sh -0x23(s0)
   *   GAME_FRONT_TIMER (0x80143C20) = sh -0x13(s0)
   * EFFECT_BUSY, PAD_STATE and POPUP_WORD are genuine standalone globals (each
   * its own `lui`) and stay declared as such. */
  volatile u8*  front_gate = &GAME_FRONT_INPUT_GATE;
  volatile u16* busy = (volatile u16*)(front_gate - 0xF3);
  volatile u16* state = (volatile u16*)(front_gate - 0x23);
  volatile u16* timer = (volatile u16*)(front_gate - 0x13);

  if (*front_gate != 0u) {
    if ((GAME_FRONT_PAD_STATE & GAME_FRONT_START_MASK) != 0u) {
      if (func_80162D00()) {
        if (GAME_FRONT_EFFECT_BUSY == 0u) {
          if (*busy == 0u) {
            if (*state < 3u) {
              *timer = 1u;
              *state = 2u;
            } else if ((GAME_FRONT_POPUP_WORD &
                        GAME_FRONT_POPUP_PENDING_MASK) ==
                       GAME_FRONT_POPUP_PENDING_OPEN) {
              func_8014ECAC(0);
              func_801D1134();
              game_queue_frontend_cue(0x105u);
              *front_gate = 0u;
              *state = 7u;
            }
          }
        }
      }
    }
  }
}
