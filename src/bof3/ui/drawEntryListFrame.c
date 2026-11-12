#include "bof3/bof3.h"

extern void drawWindowTilePiece(s16 x, s16 y, u8 piece);

/* @source 0x801D42DC
 * @behavior Draws the commu01/07 entry-list frame whose top-left corner sits
 * at (x, y) from the eight window tiles that occupy pieces 6 through 13 of the
 * six-byte sprite rect table at 0x801D4CE0: top-left, top-right, bottom-left
 * and bottom-right corners, eight repeated top and bottom edge pieces on an
 * eight-pixel horizontal step, and height divided by eight minus one repeated
 * side pieces on an eight-pixel vertical step, for a frame 0x48 pixels wide
 * that spans the height byte. Both in-image callers, func_801D4178 and
 * func_801D44E0, pass x with y one pixel higher than the entry-list origin and
 * the shared entry count 0x801455C3 times sixteen as that height, immediately
 * after filling the count lines of that list. Every coordinate is truncated to
 * sixteen bits per primitive. No other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEntryListFrame(s16 x, s16 y, u8 height) {
    s32 i;
    s32 offset;
    u8 rows;

    drawWindowTilePiece((s16)x, (s16)y, 6);
    for (i = 0; i < 8; i++) {
        offset = i * 8 + 8;
        drawWindowTilePiece((s16)(x + offset), (s16)y, 7);
        drawWindowTilePiece((s16)(x + offset), (s16)(y + height), 12);
    }
    drawWindowTilePiece((s16)(x + 0x48), (s16)y, 8);
    rows = (height >> 3) - 1;
    for (i = 0; i < rows; i++) {
        offset = i * 8 + 8;
        drawWindowTilePiece((s16)x, (s16)(y + offset), 9);
        drawWindowTilePiece((s16)(x + 0x48), (s16)(y + offset), 10);
    }
    drawWindowTilePiece((s16)x, (s16)(y + height), 11);
    drawWindowTilePiece((s16)(x + 0x48), (s16)(y + height), 13);
}
