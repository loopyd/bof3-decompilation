#include "bof3/world/area03004_internal.h"

/* @source 0x801E162C
 * @behavior AREA030 sprite and texture draw: selects graphics mode 3/1 through
 *           submitTpageDrawMode and draws sprite 0xC at (x, y) through
 *           func_801E0DCC; unless the image argument holds the 0xFFFF sentinel
 *           it then draws the texture named by the main-RAM D_80010000
 *           offset-table entry (the 0x80010000 base plus the selected halfword)
 *           at (x + 0xA, y + 8) with colour 0 and flag 0xFF through
 *           func_8014F800.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E162C(s32 x, s32 y, u16 image) {
  submitTpageDrawMode(3, 1);
  func_801E0DCC(0xC, 1, (s16)x, (s16)y);

  if (image != 0xFFFF) {
    func_8014F800((s16)(x + 0xA), (s16)(y + 8), 0, 0xFF,
                  0x80010000u + PSX_PTR(u16, 0x80010000u)[image]);
  }
}
