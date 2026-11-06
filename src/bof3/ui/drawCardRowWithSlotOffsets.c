#include "bof3/bof3.h"

extern u8 D_801DB080[]; /* @source 0x801DB080 @kind data */
extern u8 D_801DB089[]; /* @source 0x801DB089 @kind data */

extern void func_801D2CF8(s16 x, s16 y, u8 tile);

/* @source 0x801D2794
 * @behavior Draws cardRow slots through func_801D2CF8, the card-tile blitter
 *           the sibling drawThreeCardRow uses, at the left edges 0x10, 0x30,
 *           0x50 and so on, each tile 0x20 by 0x28. Slot i carries byte i of
 *           the card-id run at 0x801DB080 while i is below faceUpCount and the
 *           0xFF back id from that slot on. Its row coordinate is 0x6C except
 *           for slots 1 through offsetCount of a nonzero offsetCount, which sit
 *           on row 0x74 when byte i of the flag run at 0x801DB089 is nonzero
 *           and on row 0x64 when it is zero; slot zero and every slot past
 *           offsetCount, as well as every slot of a zero offsetCount, keep row
 *           0x6C. Nothing is written and no value is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawCardRowWithSlotOffsets(s32 slotCount, s32 faceUpCount, s32 offsetCount) {
  s32 i;
  s32 y;

  for (i = 0; i < (u8)slotCount; i++) {
    y = 0x6C;
    if (i <= (u8)offsetCount && (u8)offsetCount != 0 && i != 0) {
      y = 0x74;
      if (D_801DB089[i] == 0) {
        y = 0x64;
      }
    }

    if (i < (u8)faceUpCount) {
      func_801D2CF8((s16)(0x10 + i * 0x20), (s16)y, D_801DB080[i]);
    } else {
      func_801D2CF8((s16)(0x10 + i * 0x20), (s16)y, 0xFF);
    }
  }
}
