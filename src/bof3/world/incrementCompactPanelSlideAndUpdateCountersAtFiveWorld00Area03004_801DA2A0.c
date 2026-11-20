#include "bof3/world/area03004_internal.h"

/* @behavior increments byte 9 of the slideWork record published at the scratchpad cursor
 * 0x1F800044 and redraws the AREA030 panel at the origin 0x6C - 0x50 * that
 * slideCount through drawArea030CompactPanel; once the slideCount reaches 5 it
 * restarts it at zero and, while the record byte 6 is clear, advances the
 * shared halfword counter D_80143B92, otherwise advances the shared step byte
 * D_8014403D and clears the record dispatch byte 3; then the dim tile is
 * appended through appendDimTile, and while that restarted slideCount is
 * nonzero the inset panel rectangle 0x12 - 8 * slideCount and the encoded-text
 * run at y argument 0x15 - 8 * slideCount are submitted through
 * drawFilledPanelWithInsetBorderWorld00Area03004 and drawEncodedTextAt using
 * the halfword offset-table entry 0x40 on the 0x80010000 main-RAM base.
 * @source 0x801DA2A0
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementCompactPanelSlideAndUpdateCountersAtFiveWorld00Area03004(void) {
    u8 *slideWork;
    u8 slideCount;
    u16 *halfwordCounter;
    u8 *byteCounter;

    D_1F800044[9]++;
    drawArea030CompactPanel((s16)(0x6C - D_1F800044[9] * 0x50), 0x56);
    slideWork = D_1F800044;
    if (slideWork[9] == 5) {
        slideWork[9] = 0;
        slideWork = D_1F800044;
        if (slideWork[6] == 0) {
            halfwordCounter = &D_80143B92;
            *halfwordCounter = *halfwordCounter + 1;
        } else {
            byteCounter = &D_8014403D;
            *byteCounter = *byteCounter + 1;
            slideWork[3] = 0;
        }
    }
    appendDimTile();
    slideCount = D_1F800044[9];
    if (slideCount != 0) {
        drawFilledPanelWithInsetBorderWorld00Area03004(0x14, (0x12 - slideCount * 8) & 0xFFFE, 0x118, 0x13, 0);
        drawEncodedTextAt(0x1D, (s16)(0x15 - D_1F800044[9] * 8), 0, 0xFF,
                      0x80010000u + PSX_PTR(u16, 0x80010000u)[0x40]);
    }
}
