#include "bof3/bof3.h"

/* Shared entry count of the commu entry list: the panel height is this byte
 * times sixteen and the number of entry rows the panel fills. */
extern u8 D_801455C3; /* @source 0x801455C3 @kind data */

/* Shared clut byte that both backing rectangles of the panel take. */
extern u8 D_80144952; /* @source 0x80144952 @kind data */

/* Eight-cell commu entry names of this overlay, one run per entry row and NUL
 * padded: the first ten of the twenty runs that begin here, since the kind-grid
 * panel at 0x801D44E0 indexes the same runs from the eleventh at 0x801D4D84. */
extern u8 commuEntryNameStrings[]; /* @source 0x801D4D34 @kind data */

extern void func_801AE3F0(u16 x, u16 y, u32 width, u32 height, u32 shade,
                          u32 clut); /* @source 0x801AE3F0 */
extern void func_8014F800(s16 x, s16 y, u8 align, u32 arg3,
                          u32 arg4); /* @source 0x8014F800 */
extern void drawEntryListFrame(s16 x, s16 y, u8 height);

/* @source 0x801D4178
 * @behavior Draws the commu01/07 entry-list panel. Paints its backing rectangle
 * through func_801AE3F0 at (x, y), 0x50 wide and the shared entry count
 * 0x801455C3 times sixteen high, with shade 0x80; while the mode byte is zero
 * it first paints the inset rectangle at (x + 4, y + 5), 0x30 wide by that same
 * height, with shade 0x81. Both take the shared clut byte 0x80144952. It then
 * draws the entry-list frame through drawEntryListFrame at (x, y - 1) with the
 * height rounded down to a multiple of sixteen, and, while the entry count is
 * not zero, submits one caption per entry row through func_8014F800 at
 * (x + 8, y + 2 plus sixteen per row) with alignment seven times the mode byte
 * and cell height 8, reading the eight-cell name run of that row out of
 * 0x801D4D34. Its four in-image callers func_801D1778, func_801D1A94,
 * func_801D1E48 and func_801D2110 pass an x built from a quarter-step origin,
 * their own y and mode 0 or 1. No other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEntryListPanel(s16 x, s16 y, u8 mode) {
    s32 i;
    s32 height;
    u16 span;
    u8 *count;

    count = &D_801455C3;
    height = *count << 4;
    span = height;
    if (mode == 0) {
        func_801AE3F0((u16)(x + 4), (u16)(y + 5), 0x30, span, 0x81,
                      D_80144952);
    }
    func_801AE3F0((u16)x, (u16)y, 0x50, height | 1, 0x80, D_80144952);
    drawEntryListFrame((s16)x, (s16)(y - 1), span & 0xF0);
    i = 0;
    if (i < *count) {
        do {
            func_8014F800((s16)(x + 8), (s16)(y + 2 + i * 16), mode * 7, 8,
                          (u32)&commuEntryNameStrings[i * 8]);
            i++;
        } while (i < D_801455C3);
    }
}
