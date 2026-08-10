#include "bof3/ui/game01_internal.h"

/* @behavior draws two paired frontend label groups at the requested position;
 * the first expanded pair is optional, while both groups share selection and
 * alpha controls.
 * @source 0x801D150C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void drawLabelGroups(s16 x, s16 y, u8 selected, u8 alpha) {
  u8* primitive;
  s32 marker_x;

  if (selected != 0u) {
    marker_x = GetGraphType() == 1 ? 809 : (GetGraphType() == 2 ? 809 : 217);
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, marker_x, 0);
    appendRenderPrim(2, 12);
    primitive = drawGlyph(x, y, 15, 2, 1);
    setGlyphAlpha(primitive, alpha);
    primitive = drawGlyph((s16)(x + 224), y, 16, 2, 1);
    setGlyphAlpha(primitive, alpha);
  }

  marker_x = GetGraphType() == 1 ? 681 : (GetGraphType() == 2 ? 681 : 185);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, marker_x, 0);
  appendRenderPrim(2, 12);
  primitive = drawGlyph(x, y, 4, 2, selected);
  setGlyphAlpha(primitive, alpha);
  primitive = drawGlyph((s16)(x + 224), y, 5, 2, selected);
  setGlyphAlpha(primitive, alpha);
}
