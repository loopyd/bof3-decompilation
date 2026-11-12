#include "bof3/bof3.h"

void drawTexturedQuadAtWorkCursorPairAndDepth3eLinkedAtGridCell5d34World02Area07714_801F54B8(void);

/* @source 0x801F5464
 * @behavior Loads the work record from scratchpad pointer cell 0x1F800044,
 *           subtracts 0x2000 from word +0x38 and 0x10 from halfword +0x3E.
 *           If word +0x38 is then 0x340000, sets byte +2 to 1.
 *           Always calls the local helper at 0x801F54B8 afterward.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkFieldsAndResetStepWorld02Area07714_801F5464(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u32*)(work + 0x38) -= 0x2000;
  *(u16*)(work + 0x3E) -= 0x10;
  if (*(u32*)(work + 0x38) == 0x340000) {
    work[2] = 1;
  }
  drawTexturedQuadAtWorkCursorPairAndDepth3eLinkedAtGridCell5d34World02Area07714_801F54B8();
}
