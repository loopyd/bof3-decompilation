#include "bof3/world/area03004_internal.h"

/* @source 0x801D8D70
 * @behavior draws the two stacked AREA030 status boxes at the requested origin:
 * submits the inset panel rectangles 0x8A by 0x3A at y + 2 and 0x8A by 0x5C at
 * y + 0x42 through func_801AE3F0 with the shared CLUT-bank byte D_80144952 as
 * the trailing argument, selects graphics mode 9/1, and then repeats one box
 * body at the rows y and y + 0x40. Each box body draws the 0x25 sprite at x,
 * ten 0x26 sprites spaced eight pixels from x + 0x20, the 0x27 and 0x28 sprites
 * at x + 0x70 and x + 0x88 on the box row, the two func_801D799C text rows of
 * width 0x1E (upper box) or 0x40 (lower box) at (x, row + 0x18) and
 * (x + 0x88, row + 0x18), the 0x3C sprite at x on the row + 0x36 row, sixteen
 * 0x3D sprites spaced eight pixels from x + 8 on that row and the 0x3E sprite
 * at x + 0x88 on that row.
 * @status exact
 * @match 100.00
 * @residual none
 */

/* Both panel requests read the shared CLUT-bank byte through one local pointer
 * to 0x80144952: the original materializes that address once into a saved
 * register and reloads the byte through it for each call, which keeps the
 * parameter `y` in a stack slot for its later row uses. */
void drawStatusBoxPair(s16 x, s16 y) {
  u8* clutBank;
  s32 i;

  clutBank = &D_80144952;
  func_801AE3F0((u16)x, (u16)(y + 2), 0x8A, 0x3A, 0x80, *clutBank);
  func_801AE3F0((u16)x, (u16)(y + 0x42), 0x8A, 0x5C, 0x80, *clutBank);
  submitTpageDrawMode(9, 1);

  func_801E0DCC(0x25, 1, x, y);
  for (i = 0; i < 10; i++)
    func_801E0DCC(0x26, 1, (s16)(x + 0x20 + i * 8), y);
  func_801E0DCC(0x27, 1, (s16)(x + 0x70), y);
  func_801E0DCC(0x28, 1, (s16)(x + 0x88), y);

  func_801D799C(x, (s16)(y + 0x18), 0x1E, 0);
  func_801D799C((s16)(x + 0x88), (s16)(y + 0x18), 0x1E, 1);

  func_801E0DCC(0x3C, 1, x, (s16)(y + 0x36));
  for (i = 0; i < 16; i++)
    func_801E0DCC(0x3D, 1, (s16)(x + 8 + i * 8), (s16)(y + 0x36));
  func_801E0DCC(0x3E, 1, (s16)(x + 0x88), (s16)(y + 0x36));

  func_801E0DCC(0x25, 1, x, (s16)(y + 0x40));
  for (i = 0; i < 10; i++)
    func_801E0DCC(0x26, 1, (s16)(x + 0x20 + i * 8), (s16)(y + 0x40));
  func_801E0DCC(0x27, 1, (s16)(x + 0x70), (s16)(y + 0x40));
  func_801E0DCC(0x28, 1, (s16)(x + 0x88), (s16)(y + 0x40));

  func_801D799C(x, (s16)(y + 0x58), 0x40, 0);
  func_801D799C((s16)(x + 0x88), (s16)(y + 0x58), 0x40, 1);

  func_801E0DCC(0x3C, 1, x, (s16)(y + 0x98));
  for (i = 0; i < 16; i++)
    func_801E0DCC(0x3D, 1, (s16)(x + 8 + i * 8), (s16)(y + 0x98));
  func_801E0DCC(0x3E, 1, (s16)(x + 0x88), (s16)(y + 0x98));
}
