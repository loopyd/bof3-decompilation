#include "internal.h"

/* @behavior builds the texture-page word for the selected AREA030 graphic and
 * appends its draw-mode primitive to the current ordering-table stream.
 * @source 0x801E0C80
 */
void func_801E0C80(s32 arg0, s32 arg1) {
  s32 tpage;

  if (GetGraphType() == 1) {
    tpage = ((*(s32*)(D_801E2384 + ((arg0 & 0xff) << 4)) & 3) << 9) |
            ((*(s32*)(D_801E2388 + ((arg0 & 0xff) << 4)) & 3) << 7) |
            ((*(s32*)(D_801E2390 + ((arg0 & 0xff) << 4)) & 0x300) >> 3) |
            ((*(s32*)(D_801E238C + ((arg0 & 0xff) << 4)) & 0x3ff) >> 6);
  } else if (GetGraphType() == 2) {
    tpage = ((*(s32*)(D_801E2384 + ((arg0 & 0xff) << 4)) & 3) << 9) |
            ((*(s32*)(D_801E2388 + ((arg0 & 0xff) << 4)) & 3) << 7) |
            ((*(s32*)(D_801E2390 + ((arg0 & 0xff) << 4)) & 0x300) >> 3) |
            ((*(s32*)(D_801E238C + ((arg0 & 0xff) << 4)) & 0x3ff) >> 6);
  } else {
    tpage = ((*(s32*)(D_801E2384 + ((arg0 & 0xff) << 4)) & 3) << 7) |
            ((*(s32*)(D_801E2388 + ((arg0 & 0xff) << 4)) & 3) << 5) |
            ((*(s32*)(D_801E2390 + ((arg0 & 0xff) << 4)) & 0x100) >> 4) |
            ((*(s32*)(D_801E238C + ((arg0 & 0xff) << 4)) & 0x3ff) >> 6) |
            ((*(s32*)(D_801E2390 + ((arg0 & 0xff) << 4)) & 0x200) << 2);
  }

  SetDrawMode((DR_MODE*)D_8014598C, 0, 0, tpage, 0);
  GpuAppendPrim((u32)(u8)arg1, 0x0c);
}
