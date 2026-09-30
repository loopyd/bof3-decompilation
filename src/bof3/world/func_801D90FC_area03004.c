#include "bof3/world/area03004_internal.h"

/* @source 0x801D90FC
 * @behavior draws the AREA030 status panel at the requested origin: submits the
 * 0x8B by 0x14 inset panel rectangle through func_801AE3F0 with the shared
 * CLUT-bank byte D_80144952 as its trailing argument, selects graphics mode
 * 9/1, draws the 0x3F/0x40/0x41 header sprites eight pixels apart on the
 * requested row, eight 0x26 sprites spaced eight pixels from x + 0x28, the
 * 0x42/0x43 sprites at x + 0x68 and x + 0x78, the 0x44 sprite at x + 0x80, two
 * func_801D799C text rows of width 0x78 and 0x68 at (x, y + 0x18) and
 * (x + 0x80, y + 0x28) with the 0x46 sprite between them at x + 0x80 on the
 * y + 0x18 row, the 0x2A sprite at x on the y + 0x90 row, fifteen 0x2B sprites
 * spaced eight pixels from x + 8 on that row and finally the 0x45 sprite at
 * x + 0x80 on the y + 0x88 row.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D90FC(s16 x, s16 y) {
  s16 row;
  s32 i;

  func_801AE3F0((u16)x, (u16)(y + 2), 0x8B, 0x14, 0, D_80144952);
  submitTpageDrawMode(9, 1);

  row = y;
  func_801E0DCC(0x3F, 1, x, row);
  func_801E0DCC(0x40, 1, (s16)(x + 8), row);
  func_801E0DCC(0x41, 1, (s16)(x + 0x10), row);

  for (i = 0; i < 8; i++)
    func_801E0DCC(0x26, 1, (s16)(x + 0x28 + i * 8), y);

  func_801E0DCC(0x42, 1, (s16)(x + 0x68), y);
  func_801E0DCC(0x43, 1, (s16)(x + 0x78), y);
  func_801E0DCC(0x44, 1, (s16)(x + 0x80), y);

  func_801D799C(x, (s16)(y + 0x18), 0x78, 0);
  func_801E0DCC(0x46, 1, (s16)(x + 0x80), (s16)(y + 0x18));
  func_801D799C((s16)(x + 0x80), (s16)(y + 0x28), 0x68, 4);

  func_801E0DCC(0x2A, 1, x, (s16)(y + 0x90));

  for (i = 0; i < 15; i++)
    func_801E0DCC(0x2B, 1, (s16)(x + 8 + i * 8), (s16)(y + 0x90));

  func_801E0DCC(0x45, 1, (s16)(x + 0x80), (s16)(y + 0x88));
}
