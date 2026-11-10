#include "bof3/bof3.h"
#include "gpu/prim.h"

extern int rand(void);
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_8017A97C(void *primitive);
extern s32 func_801782FC(s32 phase);
extern s32 func_801783C8(s32 phase);

/* @source 0x801EF0B0
 * @behavior Emits eight semi-transparent POLY_G3 triangles around the work
 *           record screen halfwords 0x2E and 0x30. The radius is 32 plus the
 *           low two bits of rand; successive rim samples advance by 0x200 in
 *           the engine's 0x1000-turn phase. The centre RGB channels take work
 *           byte 0xA times fifteen and the rim channels take one. Each packet
 *           is appended under ordering-table slot 3 with size 0x1C. The caller
 *           at 0x801EED60 passes no arguments and consumes no result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairWithJitteredRadiusTintedByWorkByteATimesFifteenBmagicMagic12403_801EF0B0(void) {
    POLY_G3 *primitive;
    s32 phase;
    s32 nextPhase;
    s32 radius;
    s32 red;
    s32 green;
    s32 blue;
    s32 rim;
    u8 i;

    i = 0;
    blue = red = green = g_battle_work[0xA] * 15;
    rim = 1;
    radius = (rand() & 3) | 0x20;
    do {
        primitive = (POLY_G3 *)g_PrimCursor;
        func_8017A97C(primitive);
        SetSemiTrans(primitive, 1);
        primitive->x0 = FIELD_REF(u16, g_battle_work, 0x2Eu);
        primitive->y0 = FIELD_REF(u16, g_battle_work, 0x30u);
        phase = i << 9;
        primitive->x1 = FIELD_REF(u16, g_battle_work, 0x2Eu) +
            ((func_801782FC(phase) * radius) >> 12);
        primitive->y1 = FIELD_REF(u16, g_battle_work, 0x30u) +
            ((func_801783C8(phase) * radius) >> 12);
        nextPhase = (i + 1) << 9;
        primitive->x2 = FIELD_REF(u16, g_battle_work, 0x2Eu) +
            ((func_801782FC(nextPhase) * radius) >> 12);
        i++;
        primitive->y2 = FIELD_REF(u16, g_battle_work, 0x30u) +
            ((func_801783C8(nextPhase) * radius) >> 12);
        primitive->r0 = red;
        primitive->g0 = green;
        primitive->b0 = blue;
        primitive->r1 = rim;
        primitive->g1 = rim;
        primitive->b1 = rim;
        primitive->r2 = rim;
        primitive->g2 = rim;
        primitive->b2 = rim;
        GpuAppendPrim(3, 0x1C);
    } while (i < 8);
}
