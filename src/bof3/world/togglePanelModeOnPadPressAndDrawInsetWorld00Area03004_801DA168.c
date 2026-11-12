#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 panel pad handler: draws the compact panel at (0x6C, 0x56)
 * through drawArea030CompactPanel, then reacts to the shared pad mask
 * D_80145AA8. While its 0x5000 bits are set it plays sound cue 0x100 through
 * func_8015DF18 and inverts the mode byte at offset 6 of the work record
 * published at the scratchpad cursor 0x1F800044. While its 0x50 bits are set it
 * plays cue 0x106 and stores 1 in that mode byte when its 0x10 bit is set,
 * otherwise it plays cue 0x103, and then advances the record dispatch byte 3 in
 * both cases. It then submits the row marker at (0x70, 0x82 - 0x10 * mode)
 * through func_801647C4, appends the dim tile through appendDimTile, and draws
 * the inset panel rectangle (0x14, 0x12, 0x118, 0x13) and the texture row
 * (0x1D, 0x15) through func_801D9534 and func_8014F800 using the halfword
 * offset-table entry 0x40 on the 0x80010000 main-RAM base.
 * @source 0x801DA168
 * @status exact
 * @match 100.00
 * @residual none
 */
void togglePanelModeOnPadPressAndDrawInsetWorld00Area03004_801DA168(void) {
    u8 *work;
    u16 *pad;

    pad = &D_80145AA8;
    drawArea030CompactPanel(0x6C, 0x56);
    if ((*pad & 0x5000) != 0) {
        func_8015DF18(0x100);
        work = D_1F800044;
        work[6] ^= 1;
    }
    if ((*pad & 0x50) != 0) {
        if ((*pad & 0x10) != 0) {
            func_8015DF18(0x106);
            D_1F800044[6] = 1;
        } else {
            func_8015DF18(0x103);
        }
        D_1F800044[3]++;
    }
    func_801647C4(0x70, (u16)((0x82 - D_1F800044[6] * 0x10) & 0xFFFE), 0);
    appendDimTile();
    func_801D9534(0x14, 0x12, 0x118, 0x13, 0);
    func_8014F800(0x1D, 0x15, 0, 0xFF,
                  0x80010000u + PSX_PTR(u16, 0x80010000u)[0x40]);
}
