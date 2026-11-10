#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s32 GetGraphType(void);
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800014; /* @source 0x1F800014 @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern void func_8017AA80(void *primitive); /* @source 0x8017AA80 @kind function */

/* @source 0x801EF944
 * @behavior Draws projected flat lines for ring steps 1 through 32 whose index
 * is below work byte 9, using the signed radius at scratchpad word 0 and
 * phase steps of 0x80. Projects consecutive scratchpad vectors into each line,
 * with RGB 240, 170, 200, reduced by work byte A times 20, 14, 16 when work
 * byte 2 is 2; that state also enables semi-transparency. Queues lines and
 * enclosing draw-mode packets at ordering-table slot 5. Called without
 * arguments by 0x801EF7A0 after it publishes the radius; returns nothing.
 * @status partial
 * @match 99.46
 * @residual 184/185 instructions at the original 740-byte size; the loop
 * radius-pointer copy precedes rather than follows the flags-pointer setup.
 */
void drawProjectedPartialRingOfThirtyTwoFlatLinesTintedByWorkByteABmagicMagic11303_801EF944(void) {
    s32 graph;
    s32 *radius;
    s32 *initialRadius;
    s16 *seedEdge;
    SVECTOR *vector;
    u8 *primitive;
    s32 i;
    s32 phase;
    s32 fadeState;
    long projected;
    long flags;

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0xA5;
    } else {
        graph = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(5, 0xC);
    initialRadius = &D_1F800000;
    seedEdge = &D_1F800014;
    *seedEdge = (func_801782FC(0) * *initialRadius) >> 12;
    SPAD_REF(s16, 0x16u) = (func_801783C8(0) * *initialRadius) >> 12;
    i = 1;
    fadeState = 2;
    vector = (SVECTOR *)seedEdge;
    radius = initialRadius;
    SPAD_REF(s16, 0x18u) = 0;
    for (; i < 0x21; i++) {
        if (i < g_battle_work[9]) {
            primitive = g_PrimCursor;
            func_8017AA80(primitive);
            if (g_battle_work[2] == fadeState) {
                SetSemiTrans(primitive, 1);
            }
            RotTransPers(vector, (long *)(primitive + 8), &projected, &flags);
            phase = i << 7;
            vector->vx = (func_801782FC(phase) * *radius) >> 12;
            SPAD_REF(s16, 0x16u) = (func_801783C8(phase) * *radius) >> 12;
            SPAD_REF(s16, 0x18u) = 0;
            RotTransPers(vector, (long *)(primitive + 0xC), &projected, &flags);
            primitive[4] = 0xF0;
            primitive[5] = 0xAA;
            primitive[6] = 0xC8;
            if (g_battle_work[2] == fadeState) {
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
