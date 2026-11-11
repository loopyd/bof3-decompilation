#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Graphics-type query of the running executable, owned by the SDK map at
 * 0x8017B2B4. This payload only calls it. */
extern s32 GetGraphType(void);

/* @source 0x801D550C
 * @behavior Sprite emitter of the SHISU overlay: it asks GetGraphType for the
 *           texture page, keeping 0x8F when a query answers 1 or 2 and falling
 *           back to 0x2F otherwise, stores that page in a draw-mode packet at
 *           g_PrimCursor with dfe 0, dtd 0 and no texture window, and appends
 *           the 0xC-byte packet at ordering-table slot 1. A second read of
 *           g_PrimCursor initialises a 0x14-byte SPRT: width 0x10, height 8,
 *           CLUT 0x7800, colour bytes 0x4 through 0x6 all set to 0x10 when the
 *           fourth argument is nonzero and to 0x80 when it is zero, texture row
 *           0xD8, texture column the second byte argument shifted left by four,
 *           and the two signed coordinates from arguments 0 and 1. SetSprt then
 *           stamps the packet, SetSemiTrans marks it semi transparent with 1,
 *           and the sprite is appended at slot 1 as 0x14 bytes. The two appends
 *           are func_8014E5A0 and the other callees are PsyQ helpers or the
 *           shared graphics-type query. Both call sites of this span, at
 *           0x801D49CC and 0x801D4A2C, lie inside func_801D4754, still an asm row
 *           of this target, so the argument roles stay structural. One payload
 *           cmp proved its 0x114 bytes equal func_801DA00C at 0x801DA00C in
 *           emi/etc/shop/00, whose lift recorded exact there, and func_801D4564
 *           at 0x801D4564 in emi/etc/bate/00, still an asm row there; the body
 *           carries no target-specific literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSpritePrimTpageFromGraphTypeShisu00_801D550C(s16 arg0, s16 arg1, u8 arg2, u8 arg3) {
    SPRT* primitive;
    s32   tpage;

    if (GetGraphType() == 1) {
        tpage = 0x8F;
    } else if (GetGraphType() == 2) {
        tpage = 0x8F;
    } else {
        tpage = 0x2F;
    }

    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
    GpuAppendPrim(1, 0xC);

    primitive = (SPRT*)g_PrimCursor;
    primitive->w = 0x10;
    primitive->h = 8;
    primitive->clut = 0x7800;

    if (arg3 == 0) {
        primitive->r0 = primitive->g0 = primitive->b0 = 0x80;
    } else {
        primitive->r0 = primitive->g0 = primitive->b0 = 0x10;
    }

    primitive->v0 = 0xD8;
    primitive->u0 = arg2 * 16;
    primitive->x0 = arg0;
    primitive->y0 = arg1;

    SetSprt(primitive);
    SetSemiTrans(primitive, 1);
    GpuAppendPrim(1, 0x14);
}
