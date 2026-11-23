#include "bof3/world/area03004_internal.h"

/* @behavior Run the bounded work-pointer queue appender and draw sprites 0x15, 0x16 and
 * 0x11. Clamp work words 0x0C and 0x10 to 57..121 and 65..148, then draw the
 * width-32 cell at y 179. Enable its blinking highlight when the companion
 * lies outside the marker's 32-pixel interval. Draw the trailing cell using
 * cellVariant.
 * @source 0x801D6B28
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The published cursor cell 0x1F800044 is volatile, so the original re-reads
 * it once per access group: the x clamp, the companion clamp, the marker-mode
 * comparison and the trailing halfword read. Keeping each group's pointer in
 * its own block-scoped local reproduces the original's $a1/$a2/$v1/$v0 read
 * registers and the schedule that issues the next group's read before the
 * previous group's store. Both clamps are conditional expressions so the
 * original's separate merge register ($a0) with the pass-through copy in the
 * delay slot survives; folding them back into plain if/else assignments merges
 * the load and result registers and loses that copy. The marker mode is passed
 * as two branch calls, not a ternary argument, because the original duplicates
 * the whole argument setup in both arms.
 */
void mathMarkerPositionsRow(u8 cellVariant) {
  s32 markerX;
  s32 companionX;

  queueUnprojectedActiveWork();
  submitTpageDrawMode(0, 1);
  appendIndexedSprite(0x15, 1, 0x32, 0x96);
  appendIndexedSprite(0x16, 1, 0xBC, 0x96);
  appendIndexedSprite(0x11, 1, 0x36, 0x9C);

  {
    u8* work;
    u8* next;
    u8* check;

    work = D_1F800044;
    markerX = *(s32*)(work + 0x0C);
    markerX = (markerX < 0x39) ? 0x39 : ((markerX >= 0x7A) ? 0x79 : markerX);
    next = D_1F800044;
    *(s32*)(work + 0x0C) = markerX;

    companionX = *(s32*)(next + 0x10);
    companionX = (companionX < 0x41) ? 0x41 : ((companionX >= 0x95) ? 0x94 : companionX);
    check = D_1F800044;
    *(s32*)(next + 0x10) = companionX;

    if (*(s32*)(check + 0x10) < *(s32*)(check + 0x0C) ||
        *(s32*)(check + 0x0C) + 0x20 < *(s32*)(check + 0x10)) {
      renderOptionalBlinkingHighlight((s16)*(s32*)(check + 0x0C), 0xB3, 0x20,
                    (s8)((check[6] & 7u) << 1), 1, 1);
    } else {
      renderOptionalBlinkingHighlight((s16)*(s32*)(check + 0x0C), 0xB3, 0x20,
                    (s8)((check[6] & 7u) << 1), 1, 0);
    }
  }

  if (cellVariant == 0) {
    u8* tail0 = D_1F800044;
    renderMirroredCellQuad(*(s16*)(tail0 + 0x10), 0xB3,
                                                       0, 1);
  } else {
    u8* tail1 = D_1F800044;
    renderMirroredCellQuad(*(s16*)(tail1 + 0x10), 0xB3,
                                                       1, 1);
  }
}
