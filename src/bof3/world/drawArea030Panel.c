#include "bof3/world/area03004_internal.h"

/* @source 0x801E19CC
 * @behavior draws the AREA030 panel combination at the requested origin:
 * submits the 0xB3 by 0x42 inset panel rectangle through func_801AE3F0 with
 * the shared CLUT-bank byte D_80144952 as the trailing argument, selects
 * graphics mode 9/1, draws the 0x25 sprite at x, fifteen 0x26 sprites spaced
 * eight pixels from x + 0x20, the 0x27 and 0x28 sprites at x + 0x98 and
 * x + 0xB0, the two func_801D799C text rows of width 0x28 at (x, y + 0x18) and
 * (x + 0xB0, y + 0x18), the 0x30 sprite at x on the y + 0x40 row, twenty-one
 * 0x31 sprites spaced eight pixels from x + 8 on that row and the 0x32 sprite
 * at x + 0xB0 on that row, and finally the two main-RAM offset-table textures
 * (the 0x80010000 base plus the selected halfword) at (x + 0x3E, y + 6) and
 * (x + 6, y + 0x19) with colour 0 and flag 0xFF through func_8014F800.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawArea030Panel(s16 x, s16 y) {
  s32 i;

  func_801AE3F0((u16)x, (u16)(y + 1), 0xB3, 0x42, 0x80, D_80144952);
  submitTpageDrawMode(9, 1);
  func_801E0DCC(0x25, 1, x, y);

  for (i = 0; i < 0xF; i++)
    func_801E0DCC(0x26, 1, (s16)(x + 0x20 + i * 8), y);
  func_801E0DCC(0x27, 1, (s16)(x + 0x98), y);
  func_801E0DCC(0x28, 1, (s16)(x + 0xB0), y);

  func_801D799C(x, (s16)(y + 0x18), 0x28, 0);
  func_801D799C((s16)(x + 0xB0), (s16)(y + 0x18), 0x28, 1);

  func_801E0DCC(0x30, 1, x, (s16)(y + 0x40));
  for (i = 0; i < 0x15; i++)
    func_801E0DCC(0x31, 1, (s16)(x + 8 + i * 8), (s16)(y + 0x40));
  func_801E0DCC(0x32, 1, (s16)(x + 0xB0), (s16)(y + 0x40));

  func_8014F800((s16)(x + 0x3E), (s16)(y + 6), 0, 0xFF,
                0x80010000u + PSX_PTR(u16, 0x80010000u)[0x3C]);
  func_8014F800((s16)(x + 6), (s16)(y + 0x19), 0, 0xFF,
                0x80010000u + PSX_PTR(u16, 0x80010000u)[0x41]);
}
