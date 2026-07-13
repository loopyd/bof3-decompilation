#include "internal.h"

/* @behavior advances the two frontend window fades, promotes the fade phase
 * once both channels saturate, and draws the visible menu/window layers.
 * @source 0x801d1b00 func_801d1b00
 */
void func_801d1b00(void) {
  vu8* phase;
  s32  primary_active;
  s32  secondary_active;
  u16  alpha;

  phase = VPTR(u8, 0x80143c32u);
  primary_active = 1;
  secondary_active = 1;

  if (*phase == 1u) {
    alpha = GAME_FRONT_WINDOW_ALPHA_PRIMARY + 4u;
    GAME_FRONT_WINDOW_ALPHA_PRIMARY = alpha;
    if ((s16)alpha >= 128) {
      GAME_FRONT_WINDOW_ALPHA_PRIMARY = 128u;
      primary_active = 0;
    }

    alpha = GAME_FRONT_WINDOW_ALPHA_SECONDARY + 2u;
    GAME_FRONT_WINDOW_ALPHA_SECONDARY = alpha;
    if ((s16)alpha >= 128) {
      GAME_FRONT_WINDOW_ALPHA_SECONDARY = 128u;
      secondary_active = 0;
    }

    if (((u8)primary_active == 0u) && ((u8)secondary_active == 0u)) {
      *phase = 2u;
    }
  } else if (*phase == 2u) {
    primary_active = 0;
    secondary_active = 0;
    GAME_FRONT_WINDOW_ALPHA_PRIMARY = 128u;
    GAME_FRONT_WINDOW_ALPHA_SECONDARY = 128u;
  }

  if (*phase != 0u) {
    func_801d12cc((u8)secondary_active, *CVPTR(u8, 0x80143c28u));
    func_801d16dc(26, 24, (u8)secondary_active, *CVPTR(u8, 0x80143c28u));
    func_801d150c(-6, 28, (u8)primary_active, *CVPTR(u8, 0x80143c26u));
  }
}
