#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Graphics-type query of the running executable, owned by the SDK map at
 * 0x8017B2B4. This payload only calls it. */
extern s32 GetGraphType(void);

/* Flat-rectangle emitter of the main executable at 0x801AEBA0 (unlifted): the
 * SHISU overlay only calls it, with a clipped 16-bit rectangle and a zero flag
 * word, to fill the inner area of the frame this body builds.
 * @source 0x801AEBA0 @kind function */
extern void func_801AEBA0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);

/* @source 0x801D6090
 * @behavior Translucent frame of the SHISU overlay: it reserves the 0x10-by-0x10
 *           texture window at (0, 0xF0) at g_PrimCursor, then builds four
 *           textured quads around the rectangle whose origin is (arg0, arg1) and
 *           whose width and height are arg2 and arg3. The CLUT column of every
 *           quad is arg4 times 32 plus 0x10, the flat colour is 0xAC, each quad
 *           is appended at ordering-table slot 1 as 0x28 bytes and its texture
 *           page is 0xF, and every append is preceded by one graphics-type
 *           query. The first quad is the left edge, the second the top and
 *           bottom span, the third the right-inner span at the horizontal
 *           midpoint of arg2 and the fourth the right edge; the body then
 *           reserves a 0x100-by-0x100 texture window, installs a draw mode that
 *           references it and fills the inner rectangle from (arg0 plus 2, arg1
 *           plus 2) with width arg2 minus 4 and height arg3 minus 4 through
 *           func_801AEBA0. GetGraphType, SetDrawMode, SetPolyFT4, SetSemiTrans,
 *           GetClut, func_8014E5A0 and func_801AEBA0 are main-executable calls;
 *           the payload names this span only from the jal at 0x801D306C inside
 *           func_801D2DA8, which passes the two halfwords at 0x4 and 0x6 of the
 *           D_80148648 record, the width 0x118, the height 0x13 and the scratch
 *           byte D_80144952. One payload cmp proves its 0x54C bytes equal
 *           func_801DAB90 at 0x801DAB90 in emi/etc/shop/00, func_801D1E54 at
 *           0x801D1E54 in emi/etc/sisyou/00, func_801D50E8 at 0x801D50E8 in
 *           emi/etc/bate/00, func_801D8A40 at 0x801D8A40 in emi/etc/commu02/08,
 *           func_801F3D88 at 0x801F3D88 in emi/world00/area008/13 and
 *           func_801F765C at 0x801F765C in emi/scenario/scena12/00, so the body
 *           carries no target-specific literal. The three of those spans that
 *           were lifted stayed partial at their own gates, so they supplied
 *           statement order and spelling only.
 * @status partial
 * @match 92.92
 * @residual 315 of 339 instructions match at the exact 0x54C-byte size and the
 *           original 0x78 frame. The six remaining regions are scheduling. The
 *           first quad materialises the GetClut row constant 0x1E1 five
 *           instructions early and defers the left-edge add and the arg1/arg3
 *           reloads past the colour stores. The third quad hoists the
 *           right-inner edge chain, the clut argument and the odd-width load
 *           above its y0 store and forms half_copy plus odd_width in the same
 *           register as x where the original forms it in a temporary; its
 *           half_copy plus odd16 addend is commuted. The fourth quad stores y2
 *           and v2 and reads them back without the two reloads the original
 *           emits, because the stored values are forwarded, and both subtracts
 *           are placed with the other readback adjustments instead of after
 *           them. Re-measured worse: plain compound assignments for those two
 *           readbacks (297), the same reads through u16 and u8 views (297 and
 *           314), and a separate third-quad temporary (216).
 */
void drawTexturedFrameShisu00_801D6090(s32 arg0, s32 arg1, s32 arg2, s32 arg3, u8 arg4)
{
    RECT*     texture_window;
    POLY_FT4* primitive;
    s32       x;
    s32       bottom_y;
    s32       half_copy;
    s32       sy;
    s32       clut_x;
    s32       left_x;
    s32       bottom_v;
    u8        strip_v;
    u16       odd16;
    u16       narrow_width;
    s32       y2_read;
    u16       v2_read;
    s32       width_plus_1;
    s32       odd_width;
    s32       right_base;
    s32       two;
    s32       sx;

    texture_window = (RECT*)g_PrimCursor;
    g_PrimCursor = (u8*)(texture_window + 1);
    texture_window->y = 0xf0;
    texture_window->x = 0;
    texture_window->w = 0x10;
    texture_window->h = 0x10;

    sx = (s32)(u16)arg0;
    sy = (s32)(u16)arg1;

    if (GetGraphType() != 1) {
        GetGraphType();
    }

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, 0xf, texture_window);
    func_8014E5A0(1u, 0x0cu);

    primitive = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(primitive);
    SetSemiTrans(primitive, 1);
    clut_x = ((s32)arg4 << 5) + 0x10;
    two = 2;
    primitive->x0 = (s16)sx;
    primitive->y0 = (s16)sy;
    primitive->y1 = (s16)sy;
    primitive->x2 = (s16)sx;
    left_x = arg0 + two;
    primitive->x1 = (s16)left_x;
    primitive->x3 = (s16)left_x;
    primitive->u0 = 0u;
    primitive->v0 = 0u;
    primitive->v1 = 0u;
    primitive->u2 = 0u;
    primitive->r0 = 0xacu;
    primitive->g0 = 0xacu;
    primitive->b0 = 0xacu;
    bottom_v = arg3 + 1;
    bottom_y = arg1 + bottom_v;
    strip_v = (u8)bottom_v;
    primitive->y2 = (s16)bottom_y;
    primitive->y3 = (s16)bottom_y;
    primitive->u1 = 2u;
    primitive->v2 = strip_v;
    primitive->u3 = 2u;
    primitive->v3 = strip_v;
    primitive->y0 += two;
    primitive->y2 -= 3;
    primitive->v0 += two;
    primitive->v2 -= 3;
    primitive->clut = GetClut(clut_x, 0x1e1);

    width_plus_1 = arg2 + 1;
    narrow_width = (u16)width_plus_1;

    if (GetGraphType() != 1) {
        GetGraphType();
    }

    primitive->tpage = 0x0fu;
    func_8014E5A0(1u, 0x28u);
    x = ((s32)narrow_width - 4) >> 1;

    odd_width = (arg2 - 3) & 1;
    odd16 = (u16)odd_width;
    primitive = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = (s16)left_x;
    primitive->y0 = (s16)sy;
    primitive->x1 = (s16)(x + left_x);
    half_copy = x;
    primitive->y1 = (s16)sy;
    primitive->x2 = (s16)left_x;
    primitive->y2 = (s16)bottom_y;
    primitive->x3 = (s16)(x + left_x);
    primitive->y3 = (s16)bottom_y;
    primitive->u0 = 0u;
    primitive->v0 = 0u;
    primitive->u1 = (u8)half_copy;
    primitive->v1 = 0u;
    primitive->u2 = 0u;
    primitive->v2 = strip_v;
    primitive->u3 = (u8)half_copy;
    primitive->v3 = strip_v;
    primitive->r0 = 0xacu;
    primitive->g0 = 0xacu;
    primitive->b0 = 0xacu;
    primitive->clut = GetClut(clut_x, 0x1e1);

    if (GetGraphType() != 1) {
        GetGraphType();
    }

    primitive->tpage = 0x0fu;
    func_8014E5A0(1u, 0x28u);

    primitive = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(primitive);
    SetSemiTrans(primitive, 1);
    right_base = arg0 + half_copy;
    primitive->y0 = (s16)sy;
    primitive->x0 = (s16)(right_base + two);
    x = half_copy + odd_width;
    primitive->x1 = (s16)(right_base + (x + two));
    primitive->y1 = (s16)sy;
    primitive->x2 = (s16)(right_base + two);
    primitive->y2 = (s16)bottom_y;
    primitive->x3 = (s16)(right_base + (x + two));
    primitive->y3 = (s16)bottom_y;
    primitive->u0 = 0u;
    primitive->v0 = 0u;
    primitive->u1 = (u8)(half_copy + odd16);
    primitive->v1 = 0u;
    primitive->u2 = 0u;
    primitive->v2 = strip_v;
    primitive->u3 = (u8)(half_copy + odd16);
    primitive->v3 = strip_v;
    primitive->r0 = 0xacu;
    primitive->g0 = 0xacu;
    primitive->b0 = 0xacu;
    primitive->clut = GetClut(clut_x, 0x1e1);

    if (GetGraphType() != 1) {
        GetGraphType();
    }

    primitive->tpage = 0x0fu;
    func_8014E5A0(1u, 0x28u);

    primitive = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(primitive);
    SetSemiTrans(primitive, 1);
    primitive->y2 = (s16)bottom_y;
    y2_read = primitive->y2;
    primitive->v2 = strip_v;
    v2_read = primitive->v2;
    primitive->x0 = (s16)(arg0 + width_plus_1 - two);
    primitive->x2 = (s16)(arg0 + width_plus_1 - two);
    primitive->x1 = (s16)(arg0 + width_plus_1);
    primitive->y1 = (s16)sy;
    primitive->x3 = (s16)(arg0 + width_plus_1);
    primitive->y3 = (s16)bottom_y;
    primitive->y0 = (s16)sy;
    primitive->u0 = 0u;
    primitive->v0 = 0u;
    primitive->u1 = 2u;
    primitive->v1 = 0u;
    primitive->u2 = 0u;
    primitive->u3 = 2u;
    primitive->v3 = strip_v;
    primitive->r0 = 0xacu;
    primitive->g0 = 0xacu;
    primitive->b0 = 0xacu;
    primitive->y1 += two;
    primitive->y3 -= 3;
    primitive->y2 = (s16)(y2_read - 1);
    primitive->v1 += two;
    primitive->v2 = (u8)(v2_read - 1);
    primitive->v3 -= 3;
    primitive->clut = GetClut(clut_x, 0x1e1);

    if (GetGraphType() != 1) {
        GetGraphType();
    }

    primitive->tpage = 0x0fu;
    func_8014E5A0(1u, 0x28u);

    texture_window = (RECT*)g_PrimCursor;
    g_PrimCursor = (u8*)(texture_window + 1);
    texture_window->x = 0;
    texture_window->y = 0;
    texture_window->w = 0x100;
    texture_window->h = 0x100;

    if (GetGraphType() != 1) {
        GetGraphType();
    }

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, 0xf, texture_window);
    func_8014E5A0(1u, 0x0cu);
    func_801AEBA0((s16)left_x, (s16)(arg1 + two), (s16)(arg2 - 4), (s16)(arg3 - 4), 0);
}
