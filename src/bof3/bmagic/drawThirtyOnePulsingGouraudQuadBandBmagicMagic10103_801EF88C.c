#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s16 D_1F800006; /* @source 0x1F800006 @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern s16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern void func_80155A08(s32 x, s32 y, s32 bucket, s32 size); /* @source 0x80155A08 @kind function */

/* @source 0x801EF88C
 * @behavior Draw thirty-one semi-transparent Gouraud quads at phase increments
 *           of 64. Set draw mode to tpage 0xA5 for graphic types 1 and 2,
 *           otherwise 0x35, and link it with work words 0x34/0x38, bucket 2.
 *           Seed the outer radius from work word 0x10 plus the signed angular
 *           sample at zero times work word 0x14, shifted right twelve; the
 *           inner radius takes word 0x10. Work byte B seeds the phase, masked
 *           to six bits and scaled by 64 for each successive sample. Each
 *           pass carries the preceding outer/inner vertices forward, computes
 *           the new radius with a sample of step times 64, then computes the
 *           next vertices at the work-byte-relative phase. Clear vertex z,
 *           set all outer RGB channels to work byte A times min(step,8),
 *           and all inner RGB channels to work byte A. Project four scratchpad
 *           vectors with RotTransPers4 and link a 0x24-byte packet with the
 *           same work words and bucket. Takes no arguments and returns nothing.
 *           The wider roles of the work words and angular helpers are unknown.
 * @status partial
 * @match 88.56
 * @residual Phase-byte result allocation, preheader scheduling and loop phase-load lifetimes differ; three extra call-delay nops.
 */
void drawThirtyOnePulsingGouraudQuadBandBmagicMagic10103_801EF88C(void) {
    POLY_G4 *prim;
    SVECTOR *shape;
    s16 *radius;
    s16 *phase;
    s16 *cells;
    u16 *tint;
    u8 *work;
    s16 angle;
    s16 initial;
    s32 sample;
    s32 step;
    s32 tpage;
    long depth;
    long flag;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0xA5;
    } else {
        tpage = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0xC);

    sample = func_801782FC(0);
    work = g_battle_work;
    radius = &D_1F800000;
    *radius = *(s32 *)(work + 0x10) + ((sample * *(s32 *)(work + 0x14)) >> 12);
    D_1F800002 = *(s32 *)(work + 0x10);
    angle = work[0xB];
    D_1F800004 = angle;
    initial = (angle & 0x3F) << 6;
    D_1F800006 = initial;
    shape = (SVECTOR *)&D_1F80001C;
    shape->vx = (func_801782FC(initial) * *radius) >> 12;
    D_1F80001E = (func_801783C8(D_1F800006) * *radius) >> 12;
    D_1F80002C = (func_801782FC(D_1F800006) * D_1F800002) >> 12;
    D_1F80002E = (func_801783C8(D_1F800006) * D_1F800002) >> 12;

    step = 1;
    phase = radius + 3;
    shape--;
    cells = radius;
    tint = (u16 *)(cells + 4);
    do {
        sample = step + phase[-1];
        *phase = (sample & 0x3F) << 6;
        sample = func_801782FC(step << 6);
        work = g_battle_work;
        sample = *(s32 *)(work + 0x10) + ((sample * *(s32 *)(work + 0x14)) >> 12);
        shape[0].vx = shape[1].vx;
        shape[0].vz = 0;
        shape[0].vy = shape[1].vy;
        phase[-3] = sample;
        shape[1].vx = (func_801782FC(*phase) * phase[-3]) >> 12;
        shape[1].vy = (func_801783C8(*phase) * phase[-3]) >> 12;
        shape[1].vz = 0;
        shape[2].vz = 0;
        shape[2].vx = shape[3].vx;
        shape[2].vy = shape[3].vy;
        shape[3].vx = (func_801782FC(*phase) * phase[-2]) >> 12;
        shape[3].vy = (func_801783C8(*phase) * phase[-2]) >> 12;
        shape[3].vz = 0;
        prim = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(prim);
        SetSemiTrans(prim, 1);
        if (step < 8) {
            cells[4] = g_battle_work[0xA] * step;
            cells[5] = g_battle_work[0xA];
        } else {
            cells[4] = g_battle_work[0xA] << 3;
            cells[5] = g_battle_work[0xA];
        }
        prim->r0 = tint[0];
        prim->g0 = tint[0];
        prim->b0 = tint[0];
        prim->r1 = tint[0];
        prim->g1 = tint[0];
        prim->b1 = tint[0];
        prim->r2 = tint[1];
        prim->g2 = tint[1];
        prim->b2 = tint[1];
        prim->r3 = tint[1];
        prim->g3 = tint[1];
        prim->b3 = tint[1];
        RotTransPers4(shape, shape + 1, shape + 2, shape + 3,
                      (long *)&prim->x0, (long *)&prim->x1,
                      (long *)&prim->x2, (long *)&prim->x3, &depth, &flag);
        step++;
        func_80155A08(*(s32 *)(g_battle_work + 0x34),
                      *(s32 *)(g_battle_work + 0x38), 2, 0x24);
    } while (step < 0x20);
}
