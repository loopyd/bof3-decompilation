#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800008; /* @source 0x1F800008 @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern s16 D_1F800006; /* @source 0x1F800006 @kind data */
extern SVECTOR D_1F80001C; /* @source 0x1F80001C @kind data */
extern s16 D_1F800018; /* @source 0x1F800018 @kind data */
extern s16 D_1F800020; /* @source 0x1F800020 @kind data */
extern SVECTOR D_1F80002C; /* @source 0x1F80002C @kind data */
extern s16 D_1F800028; /* @source 0x1F800028 @kind data */
extern s16 D_1F800030; /* @source 0x1F800030 @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern void func_80155A08(s32 x, s32 y, s32 list, s32 size); /* @source 0x80155A08 @kind function */
extern void func_8017A9E0(POLY_GT4 *primitive); /* @source 0x8017A9E0 @kind function */
extern s32 GetGraphType(void);

/* @source 0x801EF1D8
 * @behavior Draw eight bands of 32 semi-transparent textured Gouraud quads.
 *           Band angles descend from 24 through 17 in steps of 0x80, with
 *           longitude samples 1 through 32 at the same spacing. Scratchpad
 *           halfword zero supplies the radius. Fade the two edges from 129
 *           toward 1 using work byte 9 minus 16 and consecutive band counts.
 *           Project four scratchpad vectors and link draw-mode and 0x34-byte
 *           quad packets to list 2 at the fourth vector times 512 plus work
 *           words 0x34 and 0x38. Texture rows descend eight per band, offset
 *           by four times work byte A; texture columns advance eight per quad.
 *           Use tpage (0,1,0x340,0x100), clut (0,0x1FA), and draw mode 0xA5
 *           for graph types 1 and 2, otherwise 0x35. The caller at 0x801EEF74
 *           supplies no arguments and consumes no return value.
 * @status partial
 * @match 74.80
 * @residual 285/381 instructions match; 1508 bytes versus 1524. Frame is
 *           0x80 rather than 0x88; scratchpad bases and narrow fade temporaries
 *           have different register allocation and scheduling.
 */
void drawEightBandsOfThirtyTwoTranslucentTexturedQuadsWithWorkByte9FadeBmagicMagic10303_801EF1D8(void)
{
    s16 *shade;
    s16 *radii;
    SVECTOR *shape;
    POLY_GT4 *prim;
    u8 *work;
    s32 band;
    s32 count;
    s32 row;
    s32 lower_row;
    s32 lower_phase;
    s32 saved_row;
    s32 i;
    s32 u;
    s32 phase;
    s32 sample;
    s32 x;
    s32 y;
    s32 graph;
    long depth;
    long flag;

    band = 24;
    count = 1;
    shade = &D_1F800006 + 1;
    radii = &D_1F800006 - 2;
    shape = &D_1F80001C - 1;
    row = 96;
    lower_row = 64;
    lower_phase = 0xB80;
    do {
        work = g_battle_work;
        D_1F800006 = 129;
        D_1F800008 = 129;
        if (count - 1 < work[9] - 16) {
            D_1F800006 = work[9] - 15 - count;
            if (D_1F800006 < 17) {
                D_1F800006 = 129 - (D_1F800006 << 3);
            } else {
                D_1F800006 = 1;
            }
        }
        if (count < g_battle_work[9] - 16) {
            shade[0] = g_battle_work[9] - 16 - count;
            if (shade[0] < 17) {
                shade[0] = 129 - (shade[0] << 3);
            } else {
                shade[0] = 1;
            }
        }
        radii[0] = (func_801783C8(lower_phase) * radii[-1]) >> 12;
        radii[1] = (func_801783C8(band << 7) * radii[-1]) >> 12;
        D_1F80001C.vx = (func_801782FC(0) * radii[0]) >> 12;
        D_1F80001E = (func_801783C8(0) * radii[0]) >> 12;
        D_1F800018 = D_1F800020 = (u32)(func_801782FC(lower_phase) * radii[-1]) >> 12;
        D_1F80002C.vx = (func_801782FC(0) * radii[1]) >> 12;
        D_1F80002E = (func_801783C8(0) * radii[1]) >> 12;
        D_1F800028 = D_1F800030 = (u32)(func_801782FC(band << 7) * radii[-1]) >> 12;
        saved_row = lower_row;
        i = 1;
        u = 16;
        do {
            phase = i << 7;
            shape[0].vx = shape[1].vx;
            shape[0].vy = shape[1].vy;
            shape[1].vx = (func_801782FC(phase) * radii[0]) >> 12;
            sample = func_801783C8(phase) * radii[0];
            shape[2].vx = shape[3].vx;
            shape[2].vy = shape[3].vy;
            shape[1].vy = sample >> 12;
            shape[3].vx = (func_801782FC(phase) * radii[1]) >> 12;
            shape[3].vy = (func_801783C8(phase) * radii[1]) >> 12;
            work = g_battle_work;
            x = (shape[3].vx << 9) + FIELD_REF(s32, work, 0x34u);
            y = (shape[3].vy << 9) + FIELD_REF(s32, work, 0x38u);
            if (GetGraphType() == 1 || GetGraphType() == 2) {
                graph = 0xA5;
            } else {
                graph = 0x35;
            }
            SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
            func_80155A08(x, y, 2, 0xC);
            prim = (POLY_GT4 *)g_PrimCursor;
            func_8017A9E0(prim);
            SetSemiTrans(prim, 1);
            RotTransPers4(shape, shape + 1, shape + 2, shape + 3,
                         (long *)&prim->x0, (long *)&prim->x1,
                         (long *)&prim->x2, (long *)&prim->x3, &depth, &flag);
            prim->tpage = GetTPage(0, 1, 0x340, 0x100);
            prim->clut = GetClut(0, 0x1FA);
            prim->u0 = i << 3;
            prim->v0 = row + (g_battle_work[0xA] << 2);
            prim->u1 = u;
            prim->v1 = row + (g_battle_work[0xA] << 2);
            prim->u2 = i << 3;
            prim->v2 = (g_battle_work[0xA] << 2) + saved_row + 40;
            prim->u3 = u;
            prim->v3 = (g_battle_work[0xA] << 2) + saved_row + 40;
            prim->r0 = shade[0];
            prim->g0 = shade[0];
            prim->b0 = shade[0];
            prim->r1 = shade[0];
            prim->g1 = shade[0];
            prim->b1 = shade[0];
            prim->r2 = shade[-1];
            prim->g2 = shade[-1];
            prim->b2 = shade[-1];
            prim->r3 = shade[-1];
            prim->g3 = shade[-1];
            prim->b3 = shade[-1];
            i++;
            u += 8;
            func_80155A08(x, y, 2, 0x34);
        } while (i < 33);
        row -= 8;
        lower_row -= 8;
        lower_phase -= 128;
        band--;
        count++;
    } while (band >= 17);
}
