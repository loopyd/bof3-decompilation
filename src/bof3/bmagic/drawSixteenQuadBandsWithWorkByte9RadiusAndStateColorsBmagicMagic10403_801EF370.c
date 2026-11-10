#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s16 D_1F800018; /* @source 0x1F800018 @kind data */
extern s16 D_1F800020; /* @source 0x1F800020 @kind data */
extern s16 D_1F800028; /* @source 0x1F800028 @kind data */
extern s16 D_1F800030; /* @source 0x1F800030 @kind data */
extern SVECTOR D_1F80002C; /* @source 0x1F80002C @kind data */
extern SVECTOR D_1F80001C; /* @source 0x1F80001C @kind data */
extern s32 GetGraphType(void);
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern void func_80155A08(s32 x, s32 y, s32 list, s32 size); /* @source 0x80155A08 @kind function */

/* @source 0x801EF370
 * @behavior Draw bands of sixteen semi-transparent Gouraud quads from sampled
 *           scratchpad vectors. Set radius to work byte 9 times 30, or to
 *           240 plus (byte 9 minus 8) times 15 when state byte 2 is at least 2.
 *           Walk bands down from 24, reduced by (byte 9 minus 8) divided by 2
 *           in those states, through 17. Sample two adjacent band angles in
 *           steps of 0x80 and sixteen azimuths in steps of 0x100, project each
 *           quad and append it at work words 0x34/0x38 with list zero. State 1
 *           shades both edges with an azimuth red ramp and band green/blue;
 *           state 2 fades the first edge by byte A against a fixed second
 *           edge; state 3 sets the first edge to 1 and fades the second.
 *           Select draw page 0x2A5 for graph types 1 and 2, else 0xB5.
 *           Takes no arguments and returns nothing. Sample helper roles
 *           and work-field roles remain structural.
 * @status partial
 * @match 68.03
 * @residual Entry base registers, frame size, saved origin and scheduling differ.
 */
void drawSixteenQuadBandsWithWorkByte9RadiusAndStateColorsBmagicMagic10403_801EF370(void)
{
    u8 *work;
    s16 *radius;
    SVECTOR *shape;
    POLY_G4 *prim;
    s32 band;
    s32 shade;
    s32 upper;
    s32 lower;
    s32 step;
    s32 ramp;
    s32 phase;
    s32 sample;
    s32 graph;
    s32 x;
    s32 y;
    long depth;
    long flag;

    work = g_battle_work;
    radius = &D_1F800000;
    *radius = work[9] * 30;
    if (work[2] >= 2) {
        *radius = (work[9] - 8) * 15 + 240;
    }
    D_1F800004 = 24;
    if (work[2] >= 2) {
        D_1F800004 = 24 - (work[9] - 8) / 2;
    }
    band = D_1F800004;
    if (band >= 17) {
        shape = &D_1F80001C - 1;
        shade = (band << 3) - 127;
        upper = band << 7;
        lower = upper - 128;
        do {
            sample = func_801782FC(0);
            D_1F80001C.vx = (sample * ((func_801783C8(lower) * SPAD_REF(s16, 0u)) >> 12)) >> 12;
            sample = func_801783C8(0);
            D_1F80001C.vy = (sample * ((func_801783C8(lower) * SPAD_REF(s16, 0u)) >> 12)) >> 12;
            D_1F800018 = D_1F800020 = (u32)(func_801782FC(lower) * SPAD_REF(s16, 0u)) >> 12;
            sample = func_801782FC(0);
            D_1F80002C.vx = (sample * ((func_801783C8(upper) * SPAD_REF(s16, 0u)) >> 12)) >> 12;
            sample = func_801783C8(0);
            D_1F80002C.vy = (sample * ((func_801783C8(upper) * SPAD_REF(s16, 0u)) >> 12)) >> 12;
            D_1F800028 = D_1F800030 = (u32)(func_801782FC(upper) * SPAD_REF(s16, 0u)) >> 12;
            step = 1;
            ramp = -112;
            do {
                phase = step << 8;
                shape[0].vx = shape[1].vx;
                shape[0].vy = shape[1].vy;
                sample = func_801782FC(phase);
                shape[1].vx = (sample * ((func_801783C8(lower) * SPAD_REF(s16, 0u)) >> 12)) >> 12;
                sample = func_801783C8(phase);
                shape[1].vy = (sample * ((func_801783C8(lower) * SPAD_REF(s16, 0u)) >> 12)) >> 12;
                shape[2].vy = shape[3].vy;
                shape[2].vx = shape[3].vx;
                sample = func_801782FC(phase);
                shape[3].vx = (sample * ((func_801783C8(upper) * SPAD_REF(s16, 0u)) >> 12)) >> 12;
                sample = func_801783C8(phase);
                shape[3].vy = (sample * ((func_801783C8(upper) * SPAD_REF(s16, 0u)) >> 12)) >> 12;
                work = g_battle_work;
                x = (shape[3].vx << 9) + FIELD_REF(s32, work, 0x34u);
                y = (shape[3].vy << 9) + FIELD_REF(s32, work, 0x38u);
                if (GetGraphType() == 1 || GetGraphType() == 2) {
                    graph = 0x2A5;
                } else {
                    graph = 0xB5;
                }
                SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
                func_80155A08(x, y, 0, 0xC);
                prim = (POLY_G4 *)g_PrimCursor;
                SetPolyG4(prim);
                SetSemiTrans(prim, 1);
                RotTransPers4(shape, shape + 1, shape + 2, shape + 3,
                             (long *)&prim->x0, (long *)&prim->x1,
                             (long *)&prim->x2, (long *)&prim->x3, &depth, &flag);
                work = g_battle_work;
                switch (work[2]) {
                case 1:
                    if (step < 8) {
                        prim->r0 = (step << 4) + 80;
                        prim->g0 = shade;
                        prim->r1 = (step << 4) + 96;
                    } else {
                        prim->r0 = -48 - ramp;
                        prim->r1 = -32 - ramp;
                        prim->g0 = shade;
                    }
                    prim->b0 = shade;
                    prim->g1 = shade;
                    prim->b1 = shade;
                    prim->r2 = prim->r0;
                    prim->g2 = prim->g0;
                    prim->b2 = prim->b0;
                    prim->r3 = prim->r1;
                    prim->g3 = prim->g1;
                    prim->b3 = prim->b1;
                    break;
                case 2:
                    prim->r0 = 81 - work[0xA] * 10;
                    prim->g0 = 17 - g_battle_work[0xA] * 2;
                    prim->r1 = prim->r0;
                    prim->b0 = 17 - g_battle_work[0xA];
                    prim->r2 = 80;
                    prim->r3 = 80;
                    prim->g1 = prim->g0;
                    prim->g2 = 16;
                    prim->b2 = 16;
                    prim->g3 = 16;
                    prim->b3 = 16;
                    prim->b1 = prim->b0;
                    break;
                case 3:
                    prim->r0 = 1;
                    prim->g0 = 1;
                    prim->b0 = 1;
                    prim->r1 = 1;
                    prim->g1 = 1;
                    prim->b1 = 1;
                    prim->r2 = 81 - g_battle_work[0xA] * 10;
                    prim->g2 = 17 - g_battle_work[0xA] * 2;
                    prim->b2 = 17 - g_battle_work[0xA] * 2;
                    prim->g3 = prim->g2;
                    prim->r3 = prim->r2;
                    prim->b3 = prim->b2;
                    break;
                }
                func_80155A08(x, y, 0, 0x24);
                step++;
                ramp += 16;
            } while (step < 17);
            shade -= 8;
            upper -= 128;
            lower -= 128;
            band--;
        } while (band >= 17);
    }
}
