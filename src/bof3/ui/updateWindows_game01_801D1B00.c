#include "bof3/ui/game01_internal.h"

/* @behavior advances the two frontend window fades, promotes the fade phase
 * once both channels saturate, and draws the visible menu/window layers.
 * @source 0x801D1B00
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
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
    /* MATCHING_AID: the original passes primary_active to drawLabelGroups's
     * selected parameter as a full word (`move a2,s1` in the jal delay slot),
     * with no andi 0xff truncation, even though the declared parameter is u8.
     * Calling through a widened s32 prototype reproduces that word-wide
     * argument pass: cc1 would otherwise narrow the u8 value with
     * `andi a2,...,0xff`. The callee (byte-matched) reads only the low byte,
     * so the wider pass is behavior-identical. */
    ((void (*)(s16, s16, s32, u8))drawLabelGroups)(-6, 28, primary_active,
                                                 D_80143C26);
  }
}
