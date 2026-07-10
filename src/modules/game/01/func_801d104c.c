#include "internal.h"

/* possible name: front_scene_pre_dispatch_gate
 * does: runs before the main GAME.EMI entry-1 state handler each frame and
 * redirects early start-button transitions into either state `2` or state `7`.
 * @source: 0x801d104c FUN_801d104c
 * @source: docs/specs/runtime/game-overlay.md
 */
void func_801d104c(void) {
  volatile u8* front_gate = (volatile u8*)0x80143c33u;

  if (*front_gate != 0u) {
    if ((GAME_FRONT_PAD_STATE & GAME_FRONT_START_MASK) != 0u) {
      if (func_80162d00()) {
        if (*(volatile u16*)(front_gate + 0x0d) == 0u) {
          if (*(volatile u16*)(front_gate - 0xf3) == 0u) {
            if (*(volatile u16*)(front_gate - 0x23) < 3u) {
              *(volatile u16*)(front_gate - 0x13) = 1u;
              *(volatile u16*)(front_gate - 0x23) = 2u;
            } else if ((*(volatile u32*)(front_gate - 3) &
                        GAME_FRONT_POPUP_PENDING_MASK) ==
                       GAME_FRONT_POPUP_PENDING_OPEN) {
              func_8014ecac(0);
              func_801d1134();
              game_queue_frontend_cue(0x105u);
              *front_gate = 0u;
              *(volatile u16*)(front_gate - 0x23) = 7u;
            }
          }
        }
      }
    }
  }
}
