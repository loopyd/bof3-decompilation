#include "bof3/world/area03004_internal.h"

/* @behavior Decrement work byte 9 modulo 256. A fresh zero read clears the work flags.
 * Otherwise select a token-cell row using work byte 0x4B and entry byte 8,
 * then draw it at x 29 with the countdown-derived width.
 * @source 0x801D41E4
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementTokenCellFlags(void) {
  u8* countdownWork;
  u8* drawWork;
  s32 remainingCount;
  u8 cellRowIndex;
  s32 tokenCellOffset;

  countdownWork = (u8*)D_1F800044;
  countdownWork[9u] = (u8)(countdownWork[9u] - 1u);

  drawWork = (u8*)D_1F800044;
  remainingCount = drawWork[9u];
  if (remainingCount != 0) {
    cellRowIndex = D_801E1E78[drawWork[0x4Bu]].byte_08;
    tokenCellOffset = cellRowIndex * 3;
    renderMessageTokenCell(D_801E1F14[tokenCellOffset + 2], &D_801E1F14[tokenCellOffset],
                  (u8)(((8 - ((4 - remainingCount) << 1)) & 0xFE) | 0x80), 29);
  } else {
    func_80196070();
  }
}
