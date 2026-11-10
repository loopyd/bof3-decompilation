#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F80000E; /* @source 0x1F80000E @kind data */
extern s32 GetGraphType(void); /* @source 0x8017B2B4 */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801EF54C
 * @behavior Draws four translucent Gouraud quads per work-byte-A segment,
 * following work words 0x0C and 0x10 from the screen pair at 0x2E and 0x30.
 * Perturbs each point using phase samples of the segment index, work byte B
 * and work word 0x14. Scratchpad halfwords retain the current and previous
 * points, radii and headings. The four layers use opposite quarter-turn
 * offsets, an eight-unit edge and a sampled-width edge. Colours use fifteen,
 * twelve and six times signed work byte 0x5D, with RGB 1 on the outer edges
 * and the first and last segment ends. Brackets the quads with graph-type
 * draw modes and appends all packets to ordering-table index 2. Called at
 * 0x801EF040 without arguments; returns nothing.
 * @status partial
 * @match 57.77
 * @residual The frame matches at 0x48 bytes, but the body is 36 bytes short.
 * Loop initialization, scratchpad base allocation, loop topology and quad
 * phase/primitive scheduling differ. Retained clean C is review-pending.
 */
void drawFourLayerTranslucentGouraudRibbonAlongWorkVelocityBmagicMagic12903_801EF54C(void) {
    POLY_G4 *prim;
    u16 *seed;
    u16 *point;
    u8 *work;
    s32 mode;
    s32 red;
    s32 green;
    s32 blue;
    s32 blue_edge;
    s32 phase;
    s32 edge_phase;
    s32 sample;
    s32 segment;
    s32 step;
    s16 radius;
    u8 i;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        mode = 0xA5;
    } else {
        mode = 0x35;
    }
    i = 1;
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
    GpuAppendPrim(2, 0xC);
    work = g_battle_work;
    seed = &D_1F800002;
    *seed = 8;
    red = FIELD_REF(s8, work, 0x5D) * 15;
    D_1F800006 = (FIELD_REF(s32, work, 0x14) + 0x800) & 0xFFF;
    green = FIELD_REF(s8, work, 0x5D) * 12;
    blue = FIELD_REF(s8, work, 0x5D) * 6;
    D_1F800008 = FIELD_REF(u16, work, 0x2E);
    D_1F80000A = FIELD_REF(u16, work, 0x30);
    blue_edge = red;
    point = seed - 1;
    for (; i < g_battle_work[0xA] + 1; i++) {
        segment = i;
        step = segment << 7;
        point[0] = point[1];
        sample = func_801782FC(step);
        point[1] = ((sample << 5) >> 12) + 8;
        point[6] = point[4];
        point[7] = point[5];
        work = g_battle_work;
        point[4] = FIELD_REF(u16, work, 0x2E) + FIELD_REF(s32, work, 0x0C) * segment;
        point[5] = FIELD_REF(u16, work, 0x30) + FIELD_REF(s32, work, 0x10) * segment;
        phase = (FIELD_REF(s32, work, 0x14) + 0x400) & 0xFFF;
        radius = (func_801782FC(step) * 48) >> 12;
        radius = (func_801782FC(((segment + g_battle_work[0xB]) & 0x3F) << 6) * radius) >> 12;
        point[4] += (func_801782FC(phase) * radius) >> 12;
        sample = func_801783C8(phase);
        point[2] = point[3];
        point[5] += (sample * radius) >> 12;
        point[3] = func_8017A138((s16)point[6] - (s16)point[4],
                               (s16)point[7] - (s16)point[5]) & 0xFFF;

        prim = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(prim);
        SetSemiTrans(prim, 1);
        edge_phase = (point[2] + 0x400) & 0xFFF;
        prim->x1 = point[6] + ((func_801782FC(edge_phase) << 3) >> 12);
        prim->y1 = point[7] + ((func_801783C8(edge_phase) << 3) >> 12);
        edge_phase = (point[3] + 0x400) & 0xFFF;
        prim->x3 = point[4] + ((func_801782FC(edge_phase) << 3) >> 12);
        prim->y3 = point[5] + ((func_801783C8(edge_phase) << 3) >> 12);
        prim->x0 = point[6];
        prim->y0 = point[7];
        prim->x2 = point[4];
        prim->y2 = point[5];
        prim->r0 = red;
        prim->g0 = red;
        prim->b0 = red;
        prim->r2 = red;
        prim->g2 = red;
        prim->b2 = red;
        prim->r1 = green;
        prim->g1 = blue;
        prim->b1 = blue_edge;
        prim->r3 = green;
        prim->g3 = blue;
        prim->b3 = blue_edge;
        if (segment == g_battle_work[0xA]) {
            prim->r2 = 1;
            prim->g2 = 1;
            prim->b2 = 1;
            prim->r3 = 1;
            prim->g3 = 1;
            prim->b3 = 1;
        }
        if (segment == 1) {
            prim->r0 = 1;
            prim->g0 = 1;
            prim->b0 = 1;
            prim->r1 = 1;
            prim->g1 = 1;
            prim->b1 = 1;
        }
        GpuAppendPrim(2, 0x24);

        prim = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(prim);
        SetSemiTrans(prim, 1);
        edge_phase = (point[2] + 0x400) & 0xFFF;
        prim->x1 = D_1F80000C + ((func_801782FC(edge_phase) * (s16)point[0]) >> 12);
        prim->y1 = D_1F80000E + ((func_801783C8(edge_phase) * (s16)point[0]) >> 12);
        prim->x0 = D_1F80000C + ((func_801782FC(edge_phase) << 3) >> 12);
        prim->y0 = D_1F80000E + ((func_801783C8(edge_phase) << 3) >> 12);
        edge_phase = (D_1F800006 + 0x400) & 0xFFF;
        prim->x3 = D_1F800008 + ((func_801782FC(edge_phase) * (s16)D_1F800002) >> 12);
        prim->y3 = D_1F80000A + ((func_801783C8(edge_phase) * (s16)D_1F800002) >> 12);
        prim->x2 = D_1F800008 + ((func_801782FC(edge_phase) << 3) >> 12);
        prim->y2 = D_1F80000A + ((func_801783C8(edge_phase) << 3) >> 12);
        prim->r0 = green;
        prim->g0 = blue;
        prim->b0 = blue_edge;
        prim->r2 = green;
        prim->g2 = blue;
        prim->b2 = blue_edge;
        prim->r1 = 1;
        prim->g1 = 1;
        prim->b1 = 1;
        prim->r3 = 1;
        prim->g3 = 1;
        prim->b3 = 1;
        if (segment == g_battle_work[0xA]) {
            prim->r2 = 1;
            prim->g2 = 1;
            prim->b2 = 1;
            prim->r3 = 1;
            prim->g3 = 1;
            prim->b3 = 1;
        }
        if (segment == 1) {
            prim->r0 = 1;
            prim->g0 = 1;
            prim->b0 = 1;
            prim->r1 = 1;
            prim->g1 = 1;
            prim->b1 = 1;
        }
        GpuAppendPrim(2, 0x24);

        prim = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(prim);
        SetSemiTrans(prim, 1);
        edge_phase = (D_1F800004 - 0x400) & 0xFFF;
        prim->x0 = D_1F80000C + ((func_801782FC(edge_phase) << 3) >> 12);
        prim->y0 = D_1F80000E + ((func_801783C8(edge_phase) << 3) >> 12);
        edge_phase = (D_1F800006 - 0x400) & 0xFFF;
        prim->x2 = D_1F800008 + ((func_801782FC(edge_phase) << 3) >> 12);
        prim->y2 = D_1F80000A + ((func_801783C8(edge_phase) << 3) >> 12);
        prim->x1 = D_1F80000C;
        prim->y1 = D_1F80000E;
        prim->x3 = D_1F800008;
        prim->y3 = D_1F80000A;
        prim->r1 = red;
        prim->g1 = red;
        prim->b1 = red;
        prim->r3 = red;
        prim->g3 = red;
        prim->b3 = red;
        prim->r0 = green;
        prim->g0 = blue;
        prim->b0 = blue_edge;
        prim->r2 = green;
        prim->g2 = blue;
        prim->b2 = blue_edge;
        if (segment == g_battle_work[0xA]) {
            prim->r2 = 1;
            prim->g2 = 1;
            prim->b2 = 1;
            prim->r3 = 1;
            prim->g3 = 1;
            prim->b3 = 1;
        }
        if (segment == 1) {
            prim->r0 = 1;
            prim->g0 = 1;
            prim->b0 = 1;
            prim->r1 = 1;
            prim->g1 = 1;
            prim->b1 = 1;
        }
        GpuAppendPrim(2, 0x24);

        prim = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(prim);
        SetSemiTrans(prim, 1);
        edge_phase = (D_1F800004 - 0x400) & 0xFFF;
        prim->x0 = D_1F80000C + ((func_801782FC(edge_phase) * (s16)point[0]) >> 12);
        prim->y0 = D_1F80000E + ((func_801783C8(edge_phase) * (s16)point[0]) >> 12);
        prim->x1 = D_1F80000C + ((func_801782FC(edge_phase) << 3) >> 12);
        prim->y1 = D_1F80000E + ((func_801783C8(edge_phase) << 3) >> 12);
        edge_phase = (D_1F800006 - 0x400) & 0xFFF;
        prim->x2 = D_1F800008 + ((func_801782FC(edge_phase) * (s16)D_1F800002) >> 12);
        prim->y2 = D_1F80000A + ((func_801783C8(edge_phase) * (s16)D_1F800002) >> 12);
        prim->x3 = D_1F800008 + ((func_801782FC(edge_phase) << 3) >> 12);
        prim->y3 = D_1F80000A + ((func_801783C8(edge_phase) << 3) >> 12);
        prim->r1 = green;
        prim->g1 = blue;
        prim->b1 = blue_edge;
        prim->r3 = green;
        prim->g3 = blue;
        prim->b3 = blue_edge;
        prim->r0 = 1;
        prim->g0 = 1;
        prim->b0 = 1;
        prim->r2 = 1;
        prim->g2 = 1;
        prim->b2 = 1;
        if (segment == g_battle_work[0xA]) {
            prim->r2 = 1;
            prim->g2 = 1;
            prim->b2 = 1;
            prim->r3 = 1;
            prim->g3 = 1;
            prim->b3 = 1;
        }
        if (segment == 1) {
            prim->r0 = 1;
            prim->g0 = 1;
            prim->b0 = 1;
            prim->r1 = 1;
            prim->g1 = 1;
            prim->b1 = 1;
        }
        GpuAppendPrim(2, 0x24);
    }
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        mode = 0x25;
    } else {
        mode = 0x15;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, mode, 0);
    GpuAppendPrim(2, 0xC);
}
