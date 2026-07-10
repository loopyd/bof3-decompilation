#include "internal.h"

/* does: picks the graph-type-specific draw mode, queues one fixed world marker,
 * then emits the textured panel primitive through `func_801f3480`.
 * @source: 0x801f33a8 FUN_801f33a8
 */
void func_801f33a8(void) {
  s16 point[3];
  u16 tpage;

  tpage = 0x225u;
  if (GetGraphType() != 1) {
    if (GetGraphType() != 2) {
      tpage = 0x95u;
    }
  }

  func_8017c2d8((void*)WORLD00_AREA027_PRIMITIVE_PTR, 0, 0, tpage, 0);
  func_80155a08(0x468000, 0x478000, -1, 0x0c);

  point[0] = (s16)0xe340u;
  point[1] = (s16)0xe3c0u;
  point[2] = 0;
  func_801f3480(point, 0x400, 0x13500126u);

  func_80155a08(0x468000, 0x478000, -1, 0x28);
}
