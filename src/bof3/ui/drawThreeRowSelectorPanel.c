#include "bof3/ui/shop00_internal.h"

/* @source 0x801D6D6C
 * @behavior draws the shop screen's fixed three-row selector panel and marks
 *           the selected row: the 0x52 by 0x48 shaded rectangle at (x + 4,
 *           y + 4) through func_801AE3F0 with the trailing constant 0 and the
 *           main-RAM CLUT-bank byte D_80144952, its frame through func_801DD350
 *           at (x, y) with the constants 9 and 8, then the three row textures
 *           through func_8014F800 at x + 0x1C and y + 0xB + slot * 0x14 with the
 *           texture handles 0xD2 + row, the constant 0 and 0xFF, and finally
 *           the selected row's marker position through the panel coordinate
 *           helper func_801647C4 at (x + 0x1C, y + 0xD + D_801E60F0 * 0x14)
 *           with the constant 0, so the overlay row index D_801E60F0 - the
 *           byte the sibling pad step func_801D3D84 walks up and down over the
 *           range 0..2 - selects the row the marker sits on. The callers pass
 *           the panel origin (0x6E, 0x4C), two of them sliding x with the frame
 *           timer.
 * @status partial
 * @match 72.97
 * @residual size and frame match, but entry copies, register allocation,
 *           loop scheduling and one commutative operand order differ.
 *           `top` preserves the full-width coordinate while `y` is narrowed
 *           for the frame call. The separate byte indices preserve the row
 *           conversions, but the build increments both indices where the
 *           original increments one and converts it in the branch delay slot.
 */
void drawThreeRowSelectorPanel(s32 x, s32 y) {
  u8 row;
  u8 slot;
  s32 left;
  s32 top;

  top = y;
  func_801AE3F0((u16)(x + 4), (u16)(y + 4), 0x52, 0x48, 0, D_80144952);
  y = (s16)y;
  func_801DD350((s16)x, y, 9, 8);
  left = x;
  slot = 0;
  for (row = 0; row < 3; row++) {
    func_8014F800((s16)(x + 0x1C), (s16)(((u32)slot * 0x14 + 0xB) + top), 0,
                  0xFF, func_801502D0(row + 0xD2));
    slot = slot + 1;
  }
  func_801647C4((u16)(left + 0x1C), (u16)(top + 0xD + D_801E60F0 * 0x14), 0);
}
