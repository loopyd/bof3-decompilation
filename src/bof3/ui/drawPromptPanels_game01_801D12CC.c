#include "bof3/ui/game01_internal.h"

/* @behavior draws both New Game/Load prompt panels, their labels and selection
 * marker; when the popup is open, pulses the active marker primitive.
 * @source 0x801D12CC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void drawPromptPanels(u8 selected, u8 alpha) {
  u8* primitive;
  s32 marker_x;
  s32 pulse;
  s32 pulse_counter;

  marker_x = GetGraphType() == 1 ? 143 : (GetGraphType() == 2 ? 143 : 47);
  SetDrawMode((DR_MODE*)D_8014598C, 0, 0, marker_x, 0);
  func_8014E5A0(1, 12);
  primitive = drawGlyph(262, 130, 1, 1, selected);
  setGlyphAlpha(primitive, alpha);

  marker_x = GetGraphType() == 1 ? 685 : (GetGraphType() == 2 ? 685 : 189);
  SetDrawMode((DR_MODE*)D_8014598C, 0, 0, marker_x, 0);
  func_8014E5A0(2, 12);
  primitive = drawGlyph(12, 200, 8, 2, selected);
  setGlyphAlpha(primitive, alpha);

  marker_x = GetGraphType() == 1 ? 685 : (GetGraphType() == 2 ? 685 : 189);
  SetDrawMode((DR_MODE*)D_8014598C, 0, 0, marker_x, 0);
  func_8014E5A0(2, 12);
  primitive = drawGlyph(12, 212, 19, 2, selected);
  setGlyphAlpha(primitive, alpha);
  primitive = drawGlyph(172, 212, 9, 2, selected);
  setGlyphAlpha(primitive, alpha);

  if ((GAME_FRONT_POPUP_WORD & GAME_FRONT_POPUP_PENDING_MASK) ==
      GAME_FRONT_POPUP_PENDING_OPEN) {
    primitive = drawGlyph(48, 184, 7, 2, 0);
    pulse_counter = D_80143C2A + 1u;
    D_80143C2A = pulse_counter;
    pulse = (pulse_counter & 0x20u) == 0u
                ? ((pulse_counter & 0x1fu) << 2)
                : -128 - ((pulse_counter & 0x1fu) << 2);
    primitive[4] = pulse;
    primitive[5] = pulse;
    primitive[6] = pulse;
  } else {
    D_80143C2A = 0u;
  }
}
