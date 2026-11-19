#include "bof3/world/area03004_internal.h"

/* @behavior advances the scratch halfword at +0x30 by eight; when the signed
 * result reaches 19 the scratch value is pinned to 18 and the dispatch index is
 * advanced. The panel request then uses the resulting halfword as its vertical
 * argument.
 * @source 0x801D3684
 * @status exact
 * @match 100.00
 * @residual none
 */
void advancePanelYAndDrawRectangleWorld00Area03004(void) {
  u8* cursor;
  u8* drawWork;
  s16* panelY;

  cursor = (u8*)D_1F800044;
  panelY = (s16*)(cursor + 0x30u);
  *panelY = (s16)(*panelY + 8);
  if (*panelY >= 19) {
    cursor[1u] = (u8)(cursor[1u] + 1u);
    *panelY = 18;
  }
  drawWork = (u8*)D_1F800044;
  func_801D9534(20, *(s16*)(drawWork + 0x30u), 280, 19, 0);
}
