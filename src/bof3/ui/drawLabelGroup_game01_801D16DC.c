#include "bof3/ui/game01_internal.h"

/* @behavior draws a paired frontend label group at the requested position,
 * applying the supplied selection and alpha controls to both primitives.
 * @source 0x801D16DC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void drawLabelGroup(s16 x, s16 y, u8 selected, u8 alpha) {
  u8* primitive;
  s32 marker_x;

  marker_x = GetGraphType() == 1 ? 683 : (GetGraphType() == 2 ? 683 : 187);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, marker_x, 0);
  appendRenderPrim(2, 12);
  primitive = drawGlyph(x, y, 2, 2, selected);
  setGlyphAlpha(primitive, alpha);
  primitive = drawGlyph((s16)(x + 240), (s16)(y + 112), 3, 2, selected);
  setGlyphAlpha(primitive, alpha);
}
