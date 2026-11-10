#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */
extern u16 D_1F800016; /* @source 0x1F800016 @kind data */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern u16 D_1F80001E; /* @source 0x1F80001E @kind data */
s32 func_801782FC(s32 phase);
s32 func_801783C8(s32 phase);
int rand(void);

/* @source 0x801EF730
 * @behavior Emit eight translucent Gouraud quads around the work record's
 * unsigned centre halfwords 0x2E and 0x30. The caller at 0x801EF284 invokes
 * this body without arguments. Seed the inner radius at scratchpad 0 from
 * work word 0x14 and the outer radius at scratchpad 2 from twice that word
 * plus rand() masked to seven. Prime the two edges at phase zero, then step
 * by 0x200 through one full 0x1000 turn. Each quad reuses the previous edge
 * and publishes the next edge in scratchpad halfwords 0x14/0x16 and 0x1C/0x1E.
 * Vertices zero and one have RGB (1,1,1); vertices two and three take the low
 * bytes of scratchpad halfwords 6, 8 and 0xA. Append each 0x24-byte packet to
 * ordering-table slot 2. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEightTranslucentGouraudRingQuadsAtWorkHalfwords2EAnd30BmagicMagic13003_801EF730(void) {
    s16 *radii;
    u16 *edge;
    u16 *seedEdge;
    s16 *outer;
    u16 *colours;
    POLY_G4 *prim;
    s32 step;
    s32 phase;

    radii = &D_1F800000;
    *radii = *(s32 *)(g_battle_work + 0x14);
    D_1F800002 = (*(s32 *)(g_battle_work + 0x14) * 2) + (rand() & 7);
    seedEdge = &D_1F800014;
    *seedEdge = *(u16 *)(g_battle_work + 0x2E) + ((func_801782FC(0) * D_1F800002) >> 12);
    D_1F800016 = *(u16 *)(g_battle_work + 0x30) + ((func_801783C8(0) * D_1F800002) >> 12);
    D_1F80001C = *(u16 *)(g_battle_work + 0x2E) + ((func_801782FC(0) * radii[0]) >> 12);
    D_1F80001E = *(u16 *)(g_battle_work + 0x30) + ((func_801783C8(0) * radii[0]) >> 12);

    step = 1;
    edge = seedEdge;
    outer = radii + 1;
    colours = (u16 *)radii;
    for (; step < 9; step++) {
        prim = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(prim);
        SetSemiTrans(prim, 1);
        prim->x0 = edge[0];
        prim->y0 = edge[1];
        prim->x2 = edge[4];
        prim->y2 = edge[5];
        phase = step << 9;
        edge[0] = *(u16 *)(g_battle_work + 0x2E) + ((func_801782FC(phase) * outer[0]) >> 12);
        prim->x1 = edge[0];
        edge[1] = *(u16 *)(g_battle_work + 0x30) + ((func_801783C8(phase) * outer[0]) >> 12);
        prim->y1 = edge[1];
        edge[4] = *(u16 *)(g_battle_work + 0x2E) + ((func_801782FC(phase) * outer[-1]) >> 12);
        prim->x3 = edge[4];
        edge[5] = *(u16 *)(g_battle_work + 0x30) + ((func_801783C8(phase) * outer[-1]) >> 12);
        prim->y3 = edge[5];
        prim->r0 = 1;
        prim->g0 = 1;
        prim->b0 = 1;
        prim->r1 = 1;
        prim->g1 = 1;
        prim->b1 = 1;
        prim->r2 = colours[3];
        prim->g2 = colours[4];
        prim->b2 = colours[5];
        prim->r3 = colours[3];
        prim->g3 = colours[4];
        prim->b3 = colours[5];
        GpuAppendPrim(2, 0x24);
    }
}
