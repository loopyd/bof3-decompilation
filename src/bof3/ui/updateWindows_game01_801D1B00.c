#include "bof3/ui/game01_internal.h"

/* @behavior advances the two frontend window fades, promotes the fade phase
 * once both channels saturate, and draws the visible menu/window layers.
 * @source 0x801D1B00
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateWindows(void) {

  u8* phase;
  u8  p;

  u8 primary_active;
  u8 secondary_active;

  s32 alpha;

  primary_active = 1;
  secondary_active = 1;
  phase = (u8*)&GAME_FRONT_WINDOW_PHASE;

  p = *phase;
  if (p == 1u) {
    alpha = GAME_FRONT_WINDOW_ALPHA_PRIMARY + 4u;
    GAME_FRONT_WINDOW_ALPHA_PRIMARY = (u16)alpha;
    if ((s16)alpha >= 128) {
      GAME_FRONT_WINDOW_ALPHA_PRIMARY = 128u;
      primary_active = 0;
    }

    alpha = GAME_FRONT_WINDOW_ALPHA_SECONDARY + 2u;
    GAME_FRONT_WINDOW_ALPHA_SECONDARY = (u16)alpha;
    if ((s16)alpha >= 128) {
      GAME_FRONT_WINDOW_ALPHA_SECONDARY = 128u;
      secondary_active = 0;
    }

    if (primary_active == 0u && secondary_active == 0u) {
      *phase = 2u;
    }
  } else if (p == 2u) {
    primary_active = 0;
    secondary_active = 0;
    GAME_FRONT_WINDOW_ALPHA_PRIMARY = 128u;
    GAME_FRONT_WINDOW_ALPHA_SECONDARY = 128u;
  }

  if (GAME_FRONT_WINDOW_PHASE != 0u) {
    drawPromptPanels(secondary_active, D_80143C28);
    drawLabelGroup(26, 24, secondary_active, D_80143C28);
    drawLabelGroups(-6, 28, primary_active, D_80143C26);
  }
}
