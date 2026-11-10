#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* @source 0x801F030C
 * @behavior Draws eight translucent Gouraud fan triangles around work
 * halfwords 0x2E and 0x30 at radius 32. Publishes the previous rim point
 * in scratchpad halfwords 0x14 and 0x16, starting at phase zero and advancing
 * by 0x200 per triangle. The centre RGB is work byte A times signed work
 * bytes 0x5D, 0x5E and 0x5F; both rim vertices have RGB 1. Links each
 * 0x1C-byte packet with work words 0x34 and 0x38 and kind 2. Called by the
 * draw path at 0x801F0054 with no arguments; returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEightTranslucentGouraudFanTrianglesRadius32TintedByWorkByteATimesSignedBytes5d5e5fBmagicMagic12903_801F030C(void) {
    POLY_G3 *primitive;
    u16 *point;
    u16 *seed;
    u8 *work;
    s32 sample;
    s32 next_y;
    s32 red;
    s32 green;
    s32 blue;
    s32 phase;
    s32 radius;
    u8 i;

    sample = func_801782FC(0);
    seed = &D_1F800014;
    *seed = FIELD_REF(u16, g_battle_work, 0x2E) + ((sample << 5) >> 12);
    sample = func_801783C8(0);
    work = g_battle_work;
    SPAD_REF(u16, 0x16) = FIELD_REF(u16, work, 0x30) + ((sample << 5) >> 12);
    red = FIELD_REF(s8, work, 0x5D) * work[0xA];
    green = FIELD_REF(s8, work, 0x5E) * work[0xA];
    blue = FIELD_REF(s8, work, 0x5F) * work[0xA];
    i = 1;
    point = seed;
    radius = 32;
    do {
        primitive = (POLY_G3 *)g_PrimCursor;
        func_8017A97C(primitive);
        SetSemiTrans(primitive, 1);
        primitive->x0 = FIELD_REF(u16, g_battle_work, 0x2E);
        primitive->y0 = FIELD_REF(u16, g_battle_work, 0x30);
        primitive->x1 = point[0];
        primitive->y1 = point[1];
        phase = i << 9;
        point[0] = FIELD_REF(u16, g_battle_work, 0x2E) +
                   ((func_801782FC(phase) * radius) >> 12);
        primitive->x2 = point[0];
        next_y = FIELD_REF(u16, g_battle_work, 0x30) +
                 ((func_801783C8(phase) * radius) >> 12);
        point[1] = next_y;
        i++;
        primitive->r0 = red;
        primitive->g0 = green;
        primitive->b0 = blue;
        primitive->r1 = 1;
        primitive->g1 = 1;
        primitive->b1 = 1;
        primitive->r2 = 1;
        primitive->g2 = 1;
        primitive->b2 = 1;
        primitive->y2 = next_y;
        func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                     FIELD_REF(s32, g_battle_work, 0x38), 2, 0x1C);
    } while (i < 9);
}
