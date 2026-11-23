#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 panel step: decrements byte 9 of the slideWork record published at
 * the scratchpad cursor 0x1F800044 and redraws the panel at the origin 0x50 *
 * count + 0x6C through drawArea030CompactPanel; once that count reaches zero
 * it arms record byte 6 and advances its dispatch byte 3; then the dim tile is
 * appended through appendDimTile, the inset panel rectangle 0x12 - 8 * count
 * is submitted through renderFilledInsetBorder and the
 * encoded-text run at y argument 0x15 - 8 * count is drawn through
 * drawEncodedTextAt using the halfword offset-table entry 0x40 on the
 * 0x80010000 main-RAM base.
 * @source 0x801DA058
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementCompactPanelSlide(void) {
    u8 *slideWork;

    slideWork = D_1F800044;
    slideWork[9] = slideWork[9] - 1;
    drawArea030CompactPanel((s16)(D_1F800044[9] * 0x50 + 0x6C), 0x56);
    slideWork = D_1F800044;
    if (slideWork[9] == 0) {
        slideWork[6] = 1;
        slideWork = D_1F800044;
        slideWork[3]++;
    }
    appendDimTile();
    renderFilledInsetBorder(0x14, (0x12 - D_1F800044[9] * 8) & 0xFFFE, 0x118, 0x13, 0);
    drawEncodedTextAt(0x1D, (s16)(0x15 - D_1F800044[9] * 8), 0, 0xFF,
                  0x80010000u + PSX_PTR(u16, 0x80010000u)[0x40]);
}
