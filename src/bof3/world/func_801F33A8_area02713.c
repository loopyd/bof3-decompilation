#include "bof3/world/area02713_internal.h"

/* @source 0x801F33A8
 * @behavior Selects the draw-mode texture depth 0x225 when the graphics type is
 *           1 or 2 and 0x95 otherwise, applies that depth to the shared
 *           primitive cursor through SetDrawMode, submits the fixed line draw of
 *           the record anchor 0x468000/0x478000 through 0x80155A08, projects
 *           that anchor minus the 0x4000 screen bias into a local
 *           three-halfword point and emits one textured marker quad for it at
 *           rotation 0x400 with texture state 0x13500126, then submits the
 *           second fixed line draw of the same anchor.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F33A8(void) {
  s16 point[3];
  s32 graph_value;
  s32 anchor_x;
  s32 anchor_y;

  anchor_x = 0x468000;
  anchor_y = 0x478000;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph_value = 0x225;
  } else {
    graph_value = 0x95;
  }

  SetDrawMode((DR_MODE*)WORLD00_AREA027_PRIMITIVE_PTR, 0, 0, graph_value, 0);
  func_80155A08(anchor_x, anchor_y, -1, 0xc);

  point[0] = (s16)(anchor_x >> 9) - 0x4000;
  point[1] = (s16)(anchor_y >> 9) - 0x4000;
  point[2] = 0;
  emitMarkerQuad(point, 0x400, 0x13500126u);

  func_80155A08(anchor_x, anchor_y, -1, 0x28);
}
