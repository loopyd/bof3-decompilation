#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern u32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern void func_8017A97C(void *prim); /* @source 0x8017A97C @kind function */
extern s32 GetGraphType(void);
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */

/* @source 0x801EF4B0
 * @behavior Draws sixteen semi-transparent Gouraud fan triangles from the
 *           origin, advancing the phase by 0x100 per slice. Scales each
 *           unsigned sampler result by three and shifts right six, carrying
 *           the previous sample into the second vertex. Clears all depths,
 *           seeds each primitive, and projects the three scratchpad vectors.
 *           Re-reads signed scratchpad word 0x1F800004 for each varying color:
 *           the origin takes that value in all channels, while each rim
 *           vertex takes it in red, half of it in green, and one in blue.
 *           Links each 0x1C-byte packet under slot 5, bracketed by 0xC-byte
 *           draw-mode packets with pages A5/35 and 25/15 according to the
 *           separately queried graph type. Takes no arguments; returns void.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSixteenTranslucentGouraudFanTrianglesWithScratchWord004TintBmagicMagic16703_801EF4B0(void) {
    u16 *next;
    s32 *tint;
    SVECTOR *shape;
    POLY_G3 *prim;
    long depth;
    long flag;
    s32 graph;
    s32 phase;
    s32 i;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0xA5;
    } else {
        graph = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(5, 0xC);

    next = &D_1F800024;
    *next = (func_801782FC(0) * 3) >> 6;
    SPAD_REF(u16, 0x26u) = (func_801783C8(0) * 3) >> 6;

    i = 1;
    shape = (SVECTOR *)(next - 8);
    tint = &D_1F800004;
    do {
        phase = i << 8;
        shape[0].vx = 0;
        shape[0].vy = 0;
        shape[1].vx = shape[2].vx;
        shape[1].vy = shape[2].vy;
        shape[2].vx = (func_801782FC(phase) * 3) >> 6;
        shape[2].vy = (func_801783C8(phase) * 3) >> 6;
        prim = (POLY_G3 *)g_PrimCursor;
        shape[2].vz = 0;
        shape[1].vz = 0;
        shape[0].vz = 0;
        func_8017A97C(prim);
        SetSemiTrans(prim, 1);
        RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                      (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);
        prim->r0 = *tint;
        prim->g0 = *tint;
        prim->b0 = *tint;
        prim->r1 = *tint;
        prim->g1 = *tint / 2;
        prim->b1 = 1;
        prim->r2 = *tint;
        prim->g2 = *tint / 2;
        prim->b2 = 1;
        GpuAppendPrim(5, 0x1C);
        i++;
    } while (i < 0x11);

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0x25;
    } else {
        graph = 0x15;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(5, 0xC);
}
