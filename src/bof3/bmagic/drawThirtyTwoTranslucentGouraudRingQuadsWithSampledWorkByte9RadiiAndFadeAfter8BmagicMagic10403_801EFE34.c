#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F800014; /* @source 0x1F800014 @kind data */
extern s32 GetGraphType(void);
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */

/* @source 0x801EFE34
 * @behavior Draw thirty-two semi-transparent Gouraud ring quads with phase
 *           steps of 0x100. Seed the two signed scratchpad radii from sampler
 *           0x801783C8 at phase 0x800 times 34 times (work byte 9 plus one)
 *           for the first radius, or times work byte 9 for the second, shifted
 *           right twelve. Carry each preceding edge point into the next
 *           quad and clear all vertex depths. Set the first edge RGB to 32
 *           while work byte 9 is at most 8, otherwise to 33 minus twice
 *           its excess over 8; set the second edge RGB to 1. Project the
 *           four scratchpad vectors with RotTransPers4 and append packets
 *           at slot 5. Use page 0xA5 for graph types 1 and 2, else 0x35,
 *           then restore page 0x25 or 0x15. Takes no arguments and returns
 *           nothing. Work-field and angular-helper roles stay structural.
 * @status partial
 * @match 88.26
 * @residual Extra saved literal register, scratchpad base reuse and scheduling differ.
 */
void drawThirtyTwoTranslucentGouraudRingQuadsWithSampledWorkByte9RadiiAndFadeAfter8BmagicMagic10403_801EFE34(void)
{
    u16 *radius;
    u16 *cells;
    SVECTOR *shape;
    POLY_G4 *prim;
    long depth;
    long flag;
    s32 graph;
    s32 phase;
    s32 step;
    s32 sample;
    u8 *work;

    sample = func_801783C8(0x800) * 34;
    work = g_battle_work;
    radius = &D_1F800000;
    *radius = (sample * (work[9] + 1)) >> 12;
    sample = func_801783C8(0x800) * 34;
    D_1F800002 = (sample * g_battle_work[9]) >> 12;
    SPAD_REF(s16, 0x1Cu) = (func_801782FC(0) * *(s16 *)radius) >> 12;
    SPAD_REF(s16, 0x1Eu) = (func_801783C8(0) * *(s16 *)radius) >> 12;
    SPAD_REF(s16, 0x2Cu) = (func_801782FC(0) * (s16)D_1F800002) >> 12;
    SPAD_REF(s16, 0x2Eu) = (func_801783C8(0) * (s16)D_1F800002) >> 12;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0xA5;
    } else {
        graph = 0x35;
    }
    step = 1;
    shape = (SVECTOR *)&D_1F800014;
    cells = radius;
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(5, 0xC);
    do {
        prim = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(prim);
        SetSemiTrans(prim, 1);
        phase = step << 8;
        shape[0].vx = shape[1].vx;
        shape[0].vy = shape[1].vy;
        shape[0].vz = 0;
        shape[1].vx = (func_801782FC(phase) * *(s16 *)cells) >> 12;
        shape[1].vy = (func_801783C8(phase) * *(s16 *)cells) >> 12;
        shape[1].vz = 0;
        shape[2].vz = 0;
        shape[2].vx = shape[3].vx;
        shape[2].vy = shape[3].vy;
        shape[3].vx = (func_801782FC(phase) * *(s16 *)(cells + 1)) >> 12;
        shape[3].vy = (func_801783C8(phase) * *(s16 *)(cells + 1)) >> 12;
        shape[3].vz = 0;
        work = g_battle_work;
        if (work[9] > 8) {
            cells[3] = 33 - ((work[9] - 8) << 1);
            prim->r0 = cells[3];
            prim->g0 = cells[3];
            prim->b0 = cells[3];
            prim->r1 = cells[3];
            prim->g1 = cells[3];
            prim->b1 = cells[3];
        } else {
            prim->r0 = 32;
            prim->g0 = 32;
            prim->b0 = 32;
            prim->r1 = 32;
            prim->g1 = 32;
            prim->b1 = 32;
        }
        prim->r2 = 1;
        prim->g2 = 1;
        prim->b2 = 1;
        prim->r3 = 1;
        prim->g3 = 1;
        prim->b3 = 1;
        RotTransPers4(shape, shape + 1, shape + 2, shape + 3,
                     (long *)&prim->x0, (long *)&prim->x1,
                     (long *)&prim->x2, (long *)&prim->x3, &depth, &flag);
        GpuAppendPrim(5, 0x24);
        step++;
    } while (step < 0x21);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0x25;
    } else {
        graph = 0x15;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(5, 0xC);
}
