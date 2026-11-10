#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* @source 0x801F05B4
 * @behavior Emits eight semi-transparent Gouraud quads around the work screen
 * pair at 0x2E and 0x30. The outer and inner radii are three and two times
 * work byte A. Outer RGB channels are one; inner RGB channels are work byte 9
 * times signed work bytes 0x5D, 0x5E and 0x5F. Successive angular samples
 * advance by 0x200. Appends each 0x24-byte packet to ordering-table slot 3.
 * Called at 0x801F0274 without arguments; no result is consumed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudRingOfEightQuadsWithWorkScaledRadiiAndTintBmagicMagic12403_801F05B4(void) {
    u8 *work;
    POLY_G4 *primitive;
    s32 phase;
    s32 nextPhase;
    s32 outerRadius;
    s32 innerRadius;
    u8 red;
    s32 green;
    s32 blue;
    s32 edge;
    u8 i;

    work = g_battle_work;
    red = work[9] * FIELD_REF(s8, work, 0x5D);
    green = work[9] * FIELD_REF(s8, work, 0x5E);
    blue = work[9] * FIELD_REF(s8, work, 0x5F);
    i = 0;
    edge = 1;
    outerRadius = work[0xA] * 3;
    innerRadius = work[0xA] * 2;
    do {
        primitive = (POLY_G4 *)g_PrimCursor;
        SetPolyG4(primitive);
        SetSemiTrans(primitive, 1);
        phase = i << 9;
        primitive->x0 = FIELD_REF(u16, g_battle_work, 0x2Eu) +
            ((func_801782FC(phase) * outerRadius) >> 12);
        primitive->y0 = FIELD_REF(u16, g_battle_work, 0x30u) +
            ((func_801783C8(phase) * outerRadius) >> 12);
        nextPhase = (i + 1) << 9;
        primitive->x1 = FIELD_REF(u16, g_battle_work, 0x2Eu) +
            ((func_801782FC(nextPhase) * outerRadius) >> 12);
        primitive->y1 = FIELD_REF(u16, g_battle_work, 0x30u) +
            ((func_801783C8(nextPhase) * outerRadius) >> 12);
        primitive->x2 = FIELD_REF(u16, g_battle_work, 0x2Eu) +
            ((func_801782FC(phase) * innerRadius) >> 12);
        primitive->y2 = FIELD_REF(u16, g_battle_work, 0x30u) +
            ((func_801783C8(phase) * innerRadius) >> 12);
        primitive->x3 = FIELD_REF(u16, g_battle_work, 0x2Eu) +
            ((func_801782FC(nextPhase) * innerRadius) >> 12);
        i++;
        primitive->y3 = FIELD_REF(u16, g_battle_work, 0x30u) +
            ((func_801783C8(nextPhase) * innerRadius) >> 12);
        primitive->r0 = edge;
        primitive->g0 = edge;
        primitive->b0 = edge;
        primitive->r1 = edge;
        primitive->g1 = edge;
        primitive->b1 = edge;
        primitive->r2 = red;
        primitive->g2 = green;
        primitive->b2 = blue;
        primitive->r3 = red;
        primitive->g3 = green;
        primitive->b3 = blue;
        GpuAppendPrim(3, 0x24);
    } while (i < 8);
}
