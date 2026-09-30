#include "bof3/world/area03004_internal.h"

/* @source 0x801D7D68
 * @behavior draws one 0x28 by 0x10 AREA030 status-panel cell at the requested
 * origin: submits the inset panel rectangle through func_801AE3F0 with the
 * shared CLUT-bank byte D_80144952 as its trailing argument, selects graphics
 * mode 8/1, then draws the flag-selected sprite (flag*4 + 0x1D) at (x, y), the
 * (flag*4 + 0x1E) sprite on the y + 0x10 row, four (flag*4 + 0x1F) sprites
 * spaced eight pixels from x + 8 on that row and finally the (flag*4 + 0x20)
 * sprite at (x + 0x28, y + 0x10).
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7D68(s16 x, s16 y, u8 flag) {
  s16 row;
  s32 sprite;
  s32 i;

  func_801AE3F0((u16)(x + 2), (u16)(y + 2), 0x28, 0x10, 0, D_80144952);
  submitTpageDrawMode(8, 1);
  sprite = flag * 4;
  func_801E0DCC((sprite + 0x1D) & 0xFF, 1, x, y);

  row = y + 0x10;
  func_801E0DCC((sprite + 0x1E) & 0xFE, 1, x, row);

  for (i = 0; i < 4; i++)
    func_801E0DCC((sprite + 0x1F) & 0xFF, 1, (s16)(x + (i + 1) * 8), row);

  /* The final cell index is recomputed from the flag instead of reusing the
   * loop's index local: the original re-derives it from the flag byte here,
   * the register holding the loop value having been consumed in place. */
  func_801E0DCC((flag * 4 + 0x20) & 0xFC, 1, (s16)(x + 0x28), (s16)(y + 0x10));
}
