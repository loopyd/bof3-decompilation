#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s32 GetGraphType(void);
extern s16 D_1F800016; /* @source 0x1F800016 @kind data */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F800020; /* @source 0x1F800020 @kind data */
extern s16 D_1F800014; /* @source 0x1F800014 @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s8 D_801F0BB8[]; /* @source 0x801F0BB8 @kind data */
extern s8 D_801F0BB9[]; /* @source 0x801F0BB9 @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern void func_8017AA80(void *primitive); /* @source 0x8017AA80 @kind function */

/* @source 0x801EFDA0
 * @behavior Draws five projected star edges, each exposed in eight increments
 * by work byte 9, from the signed phase-index run at 0x801F0BB8 with phase
 * scale 0x111 and radius 0x120. Uses RGB 240, 170, 200, reduced by work byte A
 * times 20, 14, 16 when work byte 2 is 2; that state enables semi-transparency.
 * Queues lines and enclosing draw-mode packets at ordering-table slot 5.
 * Called without arguments by 0x801EFC28; returns nothing.
 * @status partial
 * @match 89.27
 * @residual 208/232 original instructions; 932 bytes against the original 928.
 * Scratchpad vector bases and their access forms differ from the original.
 */
void drawFiveProjectedPartialStarEdgesTintedByWorkByteABmagicMagic11303_801EFDA0(void) {
    s32 graph;
    s32 i;
    s32 count;
    s32 remaining;
    SVECTOR *start;
    SVECTOR *end;
    u8 *primitive;
    long projected;
    long flags;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0xA5;
    } else {
        graph = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(5, 0xC);
    for (i = 0; i < 5; i++) {
        if ((i << 3) < g_battle_work[9]) {
            remaining = g_battle_work[9] - (i << 3);
            count = 8;
            if (remaining < 8) {
                count = remaining;
            }
            primitive = g_PrimCursor;
            func_8017AA80(primitive);
            if (g_battle_work[2] == 2) {
                SetSemiTrans(primitive, 1);
            }
            start = (SVECTOR *)&D_1F800014;
            end = (SVECTOR *)((u8 *)start + 8);
            start->vx = (func_801782FC(D_801F0BB8[i] * 0x111) * 0x120) >> 12;
            start->vy = (func_801783C8(D_801F0BB8[i] * 0x111) * 0x120) >> 12;
            start->vz = 0;
            RotTransPers(start, (long *)(primitive + 8), &projected, &flags);
            D_1F80001C = (func_801782FC(D_801F0BB9[i] * 0x111) * 0x120) >> 12;
            D_1F80001E = (func_801783C8(D_801F0BB9[i] * 0x111) * 0x120) >> 12;
            D_1F80001C = start->vx + ((D_1F80001C - start->vx) / 8) * count;
            D_1F80001E = D_1F800016 + ((D_1F80001E - D_1F800016) / 8) * count;
            D_1F800020 = 0;
            RotTransPers(end, (long *)(primitive + 0xC), &projected, &flags);
            primitive[4] = 0xF0;
            primitive[5] = 0xAA;
            primitive[6] = 0xC8;
            if (g_battle_work[2] == 2) {
                primitive[4] = 0xF0 - g_battle_work[0xA] * 20;
                primitive[5] = 0xAA - g_battle_work[0xA] * 14;
                primitive[6] = 0xC8 - g_battle_work[0xA] * 16;
            }
            GpuAppendPrim(5, 0x10);
        }
    }
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0x25;
    } else {
        graph = 0x15;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(5, 0xC);
}
