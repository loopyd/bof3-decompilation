#include "bof3/bof3.h"

extern void drawWindowTilePiece(s16 x, s16 y, u8 piece);

/* @source 0x801D3594
 * @behavior Draws the window frame whose top-left corner sits at (x, y) from
 * the eight window tiles that occupy pieces 6 through 13 of the six-byte
 * sprite rect table at 0x801D4CE0: top-left, top-right, bottom-left and
 * bottom-right corners, thirty-three repeated top and bottom edge pieces on an
 * eight-pixel horizontal step, and twenty repeated side pieces on an
 * eight-pixel vertical step, for a frame that spans 0x118 by 0xB0. The only
 * in-image caller, drawCommuEntryListScreen, passes (20, 45), one pixel up and left of
 * the 0x114 by 0xAA panel it fills just before the frame. Every coordinate is
 * truncated to sixteen bits per primitive. No other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWindowFrameCommu0107_801D3594(s16 x, s16 y) {
    s32 i;
    s32 offset;

    drawWindowTilePiece((s16)x, (s16)y, 6);
    for (i = 0; i < 33; i++) {
        offset = i * 8 + 8;
        drawWindowTilePiece((s16)(x + offset), (s16)y, 7);
        drawWindowTilePiece((s16)(x + offset), (s16)(y + 0xA8), 12);
    }
    drawWindowTilePiece((s16)(x + 0x110), (s16)y, 8);
    for (i = 0; i < 20; i++) {
        offset = i * 8 + 8;
        drawWindowTilePiece((s16)x, (s16)(y + offset), 9);
        drawWindowTilePiece((s16)(x + 0x110), (s16)(y + offset), 10);
    }
    drawWindowTilePiece((s16)x, (s16)(y + 0xA8), 11);
    drawWindowTilePiece((s16)(x + 0x110), (s16)(y + 0xA8), 13);
}
