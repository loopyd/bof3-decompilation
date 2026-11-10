#include "bof3/bof3.h"
#include "gpu/prim.h"
#include "bof3/core/slus_variables.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C @kind function */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */

/* @source 0x801EF07C
 * @behavior Draw eight semi-transparent Gouraud triangles from the origin to
 *           consecutive points at phase steps of 0x200. The radius halfword
 *           at scratchpad 0 is 128 plus the signed sample at 0x801782FC of
 *           (shared word 0x80143E6C & 31) << 7, shifted left 4 then right 12.
 *           Scratchpad halfword 8 takes six times work byte A. Each triangle
 *           uses that tint at its centre and colour (1,1,1) at its ring points.
 *           Scale sine/cosine samples by the signed radius, project the three
 *           scratchpad vectors with RotTransPers3 and append 0x1C-byte packets
 *           to ordering bucket 5. Surround the fan with 0xC-byte draw modes:
 *           texture pages 0x125/0x25 for graphic types 1 and 2, 0x55/0x15
 *           otherwise. Called by 0x801EEC18 after installing the transform.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentEightTriangleFanWithPulsingRadiusAndWorkByteATintBmagicMagic10103_801EF07C(void) {
    u16 *cells;
    u16 *next;
    u16 *tint;
    POLY_G3 *primitive;
    SVECTOR *shape;
    long depth;
    long flag;
    s32 tpage;
    s32 phase;
    s32 i;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0x125;
    } else {
        tpage = 0x55;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    GpuAppendPrim(5, 0xC);

    cells = &D_1F800000;
    next = &D_1F800024;
    *cells = ((func_801782FC((D_80143E6C & 31) << 7) << 4) >> 12) + 128;
    SPAD_REF(u16, 8) = g_battle_work[0xA] * 6;
    *next = (func_801782FC(0) * *(s16 *)cells) >> 12;
    SPAD_REF(u16, 0x26) = (func_801783C8(0) * *(s16 *)cells) >> 12;

    i = 1;
    shape = (SVECTOR *)(next - 8);
    tint = cells;
    do {
        primitive = (POLY_G3 *)g_PrimCursor;
        func_8017A97C(primitive);
        SetSemiTrans(primitive, 1);

        shape[0].vx = 0;
        shape[0].vy = 0;
        shape[1].vx = shape[2].vx;
        shape[1].vy = shape[2].vy;

        phase = i << 9;
        shape[2].vx = (func_801782FC(phase) * *(s16 *)tint) >> 12;
        shape[2].vy = (func_801783C8(phase) * *(s16 *)tint) >> 12;
        shape[2].vz = 0;
        shape[1].vz = 0;
        shape[0].vz = 0;
        RotTransPers3(shape, shape + 1, shape + 2, (long *)&primitive->x0,
                      (long *)&primitive->x1, (long *)&primitive->x2, &depth,
                      &flag);

        i++;
        primitive->r0 = tint[4];
        primitive->g0 = tint[4];
        primitive->b0 = tint[4];
        primitive->r1 = 1;
        primitive->g1 = 1;
        primitive->b1 = 1;
        primitive->r2 = 1;
        primitive->g2 = 1;
        primitive->b2 = 1;
        GpuAppendPrim(5, 0x1C);
    } while (i < 9);

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0x25;
    } else {
        tpage = 0x15;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    GpuAppendPrim(5, 0xC);
}
