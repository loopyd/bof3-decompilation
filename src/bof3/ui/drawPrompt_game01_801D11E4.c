#include "bof3/ui/game01_internal.h"

/* @behavior clears the prompt gate once the EXE reaches its idle selection
 * state, then draws the active frontend prompt and its selection marker.
 * @source 0x801D11E4
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawPrompt(void) {
  if (*(u8*)&D_80143BB0 == 5u && *(u16*)&D_80143B90 == 2u &&
      GAME_FRONT_EFFECT_BUSY == 0u) {
    D_80143C30 = 0u;
  }

  if (D_8014832E != 0u && D_80143C30 != 0u) {
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0,
                GetGraphType() == 1 ? 557 : (GetGraphType() == 2 ? 557 : 157),
                0);
    appendRenderPrim(2, 12);
    drawGlyph(192, 4, 10, 2, 0);
  }
}
