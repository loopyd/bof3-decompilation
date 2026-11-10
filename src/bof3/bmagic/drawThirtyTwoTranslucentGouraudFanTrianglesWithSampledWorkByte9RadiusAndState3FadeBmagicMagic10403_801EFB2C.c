#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */
extern s32 GetGraphType(void);
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C @kind function */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 @kind function */

/* @source 0x801EFB2C
 * @behavior Draw thirty-two semi-transparent Gouraud fan triangles. Seed the
 *           scratchpad radius with sampler 0x801783C8 at phase 0x800 times
 *           thirty times work byte 9, shifted right twelve. Seed the prior
 *           rim point at phase zero and advance by 0x100 per triangle. Clear
 *           all vertex depths. Submit draw-mode and triangle packets with
 *           kind 2 at work words 0x34 and 0x38 plus the signed rim coordinates
 *           shifted right nine. Select page 0xA5 for graph types 1 and 2,
 *           otherwise 0x35. The origin colour is (64,64,1), or
 *           (65 - 8 * work byte A, 65 - 8 * work byte A, 1) in work state 3;
 *           both rim colours are (1,1,1). Restore page 0x25 for graph types
 *           1 and 2, otherwise 0x15, and append a slot-5 draw-mode packet.
 *           No incoming argument is read and no return value is produced.
 *           Work-field roles remain offset-anchored.
 * @status partial
 * @match 88.21
 * @residual Instruction scheduling and redundant signed rim-coordinate reload.
 */
void drawThirtyTwoTranslucentGouraudFanTrianglesWithSampledWorkByte9RadiusAndState3FadeBmagicMagic10403_801EFB2C(void)
{
    u16 *radius;
    u16 *next;
    u16 *cells;
    u16 *tint;
    SVECTOR *shape;
    POLY_G3 *prim;
    u8 *work;
    long depth;
    long flag;
    s32 graph;
    s32 phase;
    s32 i;
    s32 x;
    s32 y;
    s32 sample;

    radius = &D_1F800000;
    sample = func_801783C8(0x800) * 30;
    *radius = (sample * g_battle_work[9]) >> 12;
    next = &D_1F800024;
    *next = (func_801782FC(0) * *(s16 *)radius) >> 12;
    SPAD_REF(u16, 0x26u) = (func_801783C8(0) * *(s16 *)radius) >> 12;
    i = 1;
    shape = (SVECTOR *)(next - 8);
    cells = radius;
    tint = cells + 3;
    do {
        phase = i << 8;
        shape[0].vx = 0;
        shape[0].vy = 0;
        shape[0].vz = 0;
        shape[1].vz = 0;
        shape[1].vx = shape[2].vx;
        shape[1].vy = shape[2].vy;
        shape[2].vx = (func_801782FC(phase) * *(s16 *)cells) >> 12;
        shape[2].vy = (func_801783C8(phase) * *(s16 *)cells) >> 12;
        shape[2].vz = 0;
        work = g_battle_work;
        x = (shape[2].vx >> 9) + FIELD_REF(s32, work, 0x34);
        y = (shape[2].vy >> 9) + FIELD_REF(s32, work, 0x38);
        if (GetGraphType() == 1 || GetGraphType() == 2) {
            graph = 0xA5;
        } else {
            graph = 0x35;
        }
        SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
        func_80155A08(x, y, 2, 0xC);
        prim = (POLY_G3 *)g_PrimCursor;
        func_8017A97C(prim);
        SetSemiTrans(prim, 1);
        work = g_battle_work;
        if (work[2] == 3) {
            *tint = 65 - (work[0xA] << 3);
            prim->r0 = *tint;
            prim->g0 = *tint;
            prim->b0 = 1;
        } else {
            prim->r0 = 64;
            prim->g0 = 64;
            prim->b0 = 1;
        }
        prim->r1 = 1;
        prim->g1 = 1;
        prim->b1 = 1;
        prim->r2 = 1;
        prim->g2 = 1;
        prim->b2 = 1;
        RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                     (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);
        func_80155A08(x, y, 2, 0x1C);
        i++;
    } while (i < 0x21);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        graph = 0x25;
    } else {
        graph = 0x15;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    GpuAppendPrim(5, 0xC);
}
