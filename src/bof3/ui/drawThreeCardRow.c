#include "bof3/bof3.h"

extern u8 D_801DB080[]; /* @source 0x801DB080 @kind data */

extern void func_801D2CF8(s16 x, s16 y, u8 tile);

/* @source 0x801D4F88
 * @behavior draws the three card slots that share row y with their left edges
 * at x, x + 0x20 and x + 0x40, each one 0x20 by 0x28 tile emitted through
 * func_801D2CF8. When faceUp is nonzero slot i carries the card id held in byte
 * i of the work table at 0x801DB080, whose three leading bytes the deal routine
 * at 0x801D3864 fills with unique ids drawn from the 1 through 9 range. When
 * faceUp is zero every slot carries the 0xFF id, which func_801D2CF8 renders as
 * tile 0 of the sheet through the card-back CLUT. The row is drawn left to
 * right from the unchanged base position, no state is written and nothing is
 * returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawThreeCardRow(s32 x, s32 y, u8 faceUp) {
  s32 i;

  if (faceUp) {
    for (i = 0; i < 3; i++) {
      func_801D2CF8((s16)(x + i * 0x20), (s16)y, D_801DB080[i]);
    }
  } else {
    for (i = 0; i < 3; i++) {
      func_801D2CF8((s16)(x + i * 0x20), (s16)y, 0xFF);
    }
  }
}
