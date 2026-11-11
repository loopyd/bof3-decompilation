#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Main-executable sprite emitter at 0x801AF2A0. This payload only calls it, so
 * it keeps the address-anchored spelling the generated undefined-symbol map
 * already carries for it; the id selects one of the eight border pieces 0x28
 * through 0x2F of the frame below.
 * @source 0x801AF2A0 */
extern void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* @source 0x801D78A8
 * @behavior Panel frame of the BATE overlay: takes the frame origin and the
 *           cell counts of the grid it encloses and draws the border of an
 *           8-pixel cell grid as eight border-piece sprites. It first installs
 *           the texture page GetTPage(0, 0, 0x380, 0x100) as the draw-mode
 *           packet at g_PrimCursor with dfe 0, dtd 0 and no texture window,
 *           appended at slot 1 as 0xC bytes, then emits the top and bottom
 *           edges: for every column below the column count it draws the piece
 *           0x29 at x plus (column + 1) * 8 and y, and the piece 0x2E at the
 *           same x and the bottom y that is y plus (row count + 1) * 8. The
 *           same walk then covers the left and right edges: for every row below
 *           the row count it draws the piece 0x2B at x and that row y, and the
 *           piece 0x2C at the right x that is x plus (column count + 1) * 8 and
 *           the same row y. The four corners close the frame in the order 0x28
 *           at (x, y), 0x2A at the right x and y, 0x2D at x and the bottom y,
 *           and 0x2F at the right x and the bottom y. Both counts are read as
 *           bytes, so a zero count skips its edge walk while the four corners
 *           are always drawn. The panel routine func_801D2304 of this same
 *           payload, an asm row this lift does not claim, is the one in-payload
 *           caller: the jal at 0x801D2390 loads the signed x and y into $a0 and
 *           $a1 and the constants 0x1A and 0x10 into $a2 and $a3, which fixes
 *           the argument roles as x, y, column count and row count. One payload
 *           cmp proved its 0x228 bytes equal drawPanelFrameShisu00_801D8850 at
 *           0x801D8850 in emi/etc/shisu/00, a lift recorded exact there, and
 *           func_801DD350 at 0x801DD350 in emi/etc/shop/00, all 138 words
 *           equal, and the body carries no target-specific literal, so the
 *           SHISU copy supplied its statement order and role name. Its
 *           0x38-byte frame keeps the seven saved registers s0 through s6 and
 *           $ra, holds the SetDrawMode texture window word at 0x10, and the
 *           epilogue returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawPanelFrameBate00_801D78A8(s16 x, s16 y, u8 w, u8 h) {
    u8 i;

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x380, 0x100), 0);
    GpuAppendPrim(1, 0xC);

    for (i = 0; i < w; i++) {
        func_801AF2A0(x + (i + 1) * 8, y, 0x29, 0);
        func_801AF2A0(x + (i + 1) * 8, y + (h + 1) * 8, 0x2E, 0);
    }

    for (i = 0; i < h; i++) {
        func_801AF2A0(x, y + (i + 1) * 8, 0x2B, 0);
        func_801AF2A0(x + (w + 1) * 8, y + (i + 1) * 8, 0x2C, 0);
    }

    func_801AF2A0(x, y, 0x28, 0);
    func_801AF2A0(x + (w + 1) * 8, y, 0x2A, 0);
    func_801AF2A0(x, y + (h + 1) * 8, 0x2D, 0);
    func_801AF2A0(x + (w + 1) * 8, y + (h + 1) * 8, 0x2F, 0);
}
