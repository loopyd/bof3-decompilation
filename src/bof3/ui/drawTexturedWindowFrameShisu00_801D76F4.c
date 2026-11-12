#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Main-executable sprite emitter at 0x801AF2A0, whose body the emi/etc/game/00
 * overlay holds at that same address under the name drawSprite: it fills one GT
 * quad from the sprite-record table entry the id selects and appends it at slot
 * 1 as 0x14 bytes. This payload only calls it, so it keeps the address-anchored
 * spelling the generated undefined-symbol map already carries for it.
 * @source 0x801AF2A0 */
extern void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* @source 0x801D76F4
 * @behavior Textured window frame of the SHISU overlay: it takes the frame
 *           origin and the cell counts of the rectangle it encloses, draws the
 *           four edges and the inner area as five stretched sprites and then the
 *           four corner pieces. Every sprite is preceded by its own draw mode:
 *           the texture page GetTPage(0, 0, 0x340, 0x100) becomes the tpage word
 *           of a packet at g_PrimCursor with dfe 0 and dtd 0, appended at slot 1
 *           as 0xC bytes, and the texture window RECT that the same call
 *           installs is an 8-by-8 rectangle whose origin selects one atlas cell
 *           for that sprite. The first sprite is the top edge, with the window
 *           at (0x70, 0x98), the origin (x plus 0x10, y), the width (cellW minus
 *           4) times 8 and the height 8. The second is the bottom edge, with the
 *           window at (0x98, 0xA0), the origin (x plus 0x10, y plus (cellH minus
 *           1) times 8), the same width and the height 8. The third is the left
 *           edge, with the window at (0xB0, 0x98), the origin (x, y plus 0x10),
 *           the width 8 and the height (cellH minus 4) times 8. The fourth is
 *           the right edge, with the window at (0xB0, 0xA0), the origin (x plus
 *           (cellW minus 1) times 8, y plus 0x10), the width 8 and the same
 *           height. The fifth is the inner area, with the window at
 *           (0x70, 0xA0), the origin (x plus 8, y plus 8), the width (cellW
 *           minus 2) times 8 and the height (cellH minus 2) times 8. Every
 *           sprite is stamped by SetSprt, carries the CLUT GetClut(0xB0, 0x1E1),
 *           the texture column and row 0 and the three colour bytes 0x80, and is
 *           appended at slot 1 as 0x14 bytes. A sixth draw mode then restores
 *           the whole-page window (0, 0, 0x100, 0x100) and appends its 0xC-byte
 *           packet without a sprite. The four corner pieces follow through
 *           func_801AF2A0 with the flags word 1: 0x23 at (x, y), 0x26 at (x plus
 *           (cellW minus 2) times 8, y), 0x27 at (x, y plus (cellH minus 2)
 *           times 8) and 0x29 at both of those offsets, each argument the low
 *           halfword of that sum because x and y are signed words, while cellW
 *           and cellH are read as bytes. No jal and no table word of this
 *           payload names this body, so it is an overlay entry called from
 *           outside the payload and its argument roles are the ones its own
 *           arithmetic shows. One payload cmp proved its 0x508 bytes equal
 *           func_801DC1F4 at 0x801DC1F4 in emi/etc/shop/00 and the span at
 *           0x801D674C in emi/etc/bate/00 word for word, so the body carries no
 *           target-specific literal or binding; both of those rows are still asm
 *           rows, so neither carries a source to clone. Its 0x80-byte frame
 *           keeps s0 through s7, $fp and $ra, holds the texture window RECT at
 *           0x18, the byte copies of the two counts at 0x28 and 0x30 and the
 *           texture window pointer at 0x50, and the epilogue returns no value.
 * @status partial
 * @match 70.19
 * @residual The body reaches 1280 of the original 1288 bytes and matches 226 of
 *           its 322 instructions. The unclosed remainder is one allocation fact
 *           and the register set it drags behind it: the original keeps both
 *           byte counts and the two-word span product in stack slots, which is
 *           what its 0x80 frame holds at 0x28, 0x30, 0x40 and 0x48 beside the
 *           window pointer at 0x50, and keeps the left-edge origin sum y plus
 *           0x10 in a saved register, while every shape measured here holds the
 *           counts in callee-saved registers behind one andi mask and spills
 *           that sum instead, at a 0x68 frame with the same texture window
 *           offsets. Measured spellings that all reproduce the swap: the inline
 *           expression form (1264 bytes), s16 and u16 origin parameters (1268),
 *           u8 count locals and s32 count parameters with u8 locals (1264),
 *           named span products assigned at the top (1280, this candidate) or
 *           lazily before their block (1264), a pointer local for the window
 *           (1264), and the constant side of each sprite stored first (1264 to
 *           1280).
 */
void drawTexturedWindowFrameShisu00_801D76F4(s32 x, s32 y, u8 cellW, u8 cellH) {
    SPRT* prim;
    RECT  texture_window;
    s32   edgeW;
    s32   edgeH;
    s32   innerW;
    s32   innerH;
    s32   sideY;
    s32   rightX;

    edgeW = (cellW - 4) * 8;
    edgeH = (cellH - 4) * 8;
    innerW = (cellW - 2) * 8;
    innerH = (cellH - 2) * 8;
    sideY = y + 0x10;
    rightX = x + (cellW - 1) * 8;

    texture_window.x = 112;
    texture_window.y = 152;
    texture_window.w = 8;
    texture_window.h = 8;
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x340, 0x100),
                &texture_window);
    GpuAppendPrim(1, 0xC);

    prim = (SPRT*)g_PrimCursor;
    SetSprt(prim);
    prim->x0 = x + 0x10;
    prim->y0 = y;
    prim->u0 = 0;
    prim->v0 = 0;
    prim->w = edgeW;
    prim->h = 8;
    prim->clut = GetClut(0xB0, 0x1E1);
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    GpuAppendPrim(1, 0x14);

    texture_window.x = 152;
    texture_window.y = 160;
    texture_window.w = 8;
    texture_window.h = 8;
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x340, 0x100),
                &texture_window);
    GpuAppendPrim(1, 0xC);

    prim = (SPRT*)g_PrimCursor;
    SetSprt(prim);
    prim->x0 = x + 0x10;
    prim->y0 = y + (cellH - 1) * 8;
    prim->u0 = 0;
    prim->v0 = 0;
    prim->w = edgeW;
    prim->h = 8;
    prim->clut = GetClut(0xB0, 0x1E1);
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    GpuAppendPrim(1, 0x14);

    texture_window.x = 176;
    texture_window.y = 152;
    texture_window.w = 8;
    texture_window.h = 8;
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x340, 0x100),
                &texture_window);
    GpuAppendPrim(1, 0xC);

    prim = (SPRT*)g_PrimCursor;
    SetSprt(prim);
    prim->x0 = x;
    prim->y0 = sideY;
    prim->u0 = 0;
    prim->v0 = 0;
    prim->w = 8;
    prim->h = edgeH;
    prim->clut = GetClut(0xB0, 0x1E1);
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    GpuAppendPrim(1, 0x14);

    texture_window.x = 176;
    texture_window.y = 160;
    texture_window.w = 8;
    texture_window.h = 8;
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x340, 0x100),
                &texture_window);
    GpuAppendPrim(1, 0xC);

    prim = (SPRT*)g_PrimCursor;
    SetSprt(prim);
    prim->x0 = rightX;
    prim->y0 = sideY;
    prim->u0 = 0;
    prim->v0 = 0;
    prim->w = 8;
    prim->h = edgeH;
    prim->clut = GetClut(0xB0, 0x1E1);
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    GpuAppendPrim(1, 0x14);

    texture_window.x = 112;
    texture_window.y = 160;
    texture_window.w = 8;
    texture_window.h = 8;
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x340, 0x100),
                &texture_window);
    GpuAppendPrim(1, 0xC);

    prim = (SPRT*)g_PrimCursor;
    SetSprt(prim);
    prim->x0 = x + 8;
    prim->y0 = y + 8;
    prim->u0 = 0;
    prim->v0 = 0;
    prim->w = innerW;
    prim->h = innerH;
    prim->clut = GetClut(0xB0, 0x1E1);
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    GpuAppendPrim(1, 0x14);

    texture_window.x = 0;
    texture_window.y = 0;
    texture_window.w = 0x100;
    texture_window.h = 0x100;
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 0, 0x340, 0x100),
                &texture_window);
    GpuAppendPrim(1, 0xC);

    func_801AF2A0((s16)x, (s16)y, 0x23, 1);
    func_801AF2A0((s16)(x + innerW), (s16)y, 0x26, 1);
    func_801AF2A0((s16)x, (s16)(y + innerH), 0x27, 1);
    func_801AF2A0((s16)(x + innerW), (s16)(y + innerH), 0x29, 1);
}
