#include "bof3/world/area03004_internal.h"

/* @source 0x801D7784
 * @behavior draws the AREA030 status panel icon row and its framed sprite
 * columns at the requested origin: selects graphics mode 9/1, appends sprite
 * 0x25 at (x, y), 0x17 sprites 0x26 spaced 8 pixels from x + 0x20, 0x27 at
 * x + 0xD8 and 0x28 at x + 0xF0, submits the frame quads at x and x + 0xF0
 * through func_801D799C, then draws 0x2A at x, 0x17 sprites 0x2B from x + 8,
 * 0x2C at x + 0xC0, three sprites 0x2D from x + 0xC8 and finally 0x2E at
 * x + 0xE0 and 0x2F at x + 0xE7, all on the y + 0x90 row.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7784(s16 x, s16 y) {
  s32 i;

  submitTpageDrawMode(9, 1);
  func_801E0DCC(0x25, 1, x, y);

  for (i = 0; i < 0x17; i++)
    func_801E0DCC(0x26, 1, (s16)(x + 0x20 + i * 8), y);

  func_801E0DCC(0x27, 1, (s16)(x + 0xD8), y);
  func_801E0DCC(0x28, 1, (s16)(x + 0xF0), y);
  func_801D799C(x, (s16)(y + 0x18), 0x78, 0);
  func_801D799C((s16)(x + 0xF0), (s16)(y + 0x18), 0x78, 1);
  func_801E0DCC(0x2A, 1, x, (s16)(y + 0x90));

  for (i = 0; i < 0x17; i++)
    func_801E0DCC(0x2B, 1, (s16)(x + 8 + i * 8), (s16)(y + 0x90));

  func_801E0DCC(0x2C, 1, (s16)(x + 0xC0), (s16)(y + 0x90));

  for (i = 0; i < 3; i++)
    func_801E0DCC(0x2D, 1, (s16)(x + 0xC8 + i * 8), (s16)(y + 0x90));

  func_801E0DCC(0x2E, 1, (s16)(x + 0xE0), (s16)(y + 0x90));
  func_801E0DCC(0x2F, 1, (s16)(x + 0xE7), (s16)(y + 0x90));
}
