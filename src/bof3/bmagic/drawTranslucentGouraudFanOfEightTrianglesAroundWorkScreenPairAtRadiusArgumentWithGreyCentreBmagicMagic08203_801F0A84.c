#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */
extern s32 GetGraphType(void);

/* The fan shape follows the original span and the eight-triangle drawing
 * sources in magic097/03 and magic092/03. The magic083/03 span at 0x801F0BA8
 * differs only in its relocated internal jump; it supplies shape, not clone
 * evidence. The shared helpers retain their address-anchored names.
 */
/* @source 0x801F0A84
 * @behavior Draw eight translucent Gouraud triangles around work halfwords
 *           0x2E and 0x30. Store the argument's low halfword in scratchpad
 *           0x1F800000 and use its signed value as the radius. Each iteration
 *           emits a draw-mode packet with tpage 0xA5 for graph type 1 or 2,
 *           otherwise 0x35, then a triangle joining the centre and consecutive
 *           phase samples spaced by 0x200. The centre is grey 0x80 and both
 *           rim vertices have RGB 1. Link the 0xC-byte and 0x1C-byte packets
 *           through helper 0x80155A08 using work words 0x34 and 0x38, kind 2.
 *           The four original callers at 0x801EFEF4, 0x801EFFE8, 0x801F00DC
 *           and 0x801F0260 pass the radius in a0; no return value is consumed.
 * @status partial
 * @match 93.28
 * @residual 125/134 instructions; 532/536 bytes. The radius base is formed
 *           directly in its saved register rather than copied from v0.
 */
void drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairAtRadiusArgumentWithGreyCentreBmagicMagic08203_801F0A84(
    s32 radius_arg) {
  POLY_G3 *primitive;
  s16 *radius;
  s32 phase;
  s32 next_i;
  s32 next_phase;
  s32 tpage;
  s32 i;

  radius = &D_1F800000;
  *radius = radius_arg;
  i = 0;
  do {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0xA5;
    } else {
      tpage = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0xC);

    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = FIELD_REF(u16, g_battle_work, 0x2Eu);
    primitive->y0 = FIELD_REF(u16, g_battle_work, 0x30u);
    phase = i << 9;
    primitive->x1 =
        FIELD_REF(u16, g_battle_work, 0x2Eu) +
        ((func_801782FC(phase) * *radius) >> 12);
    primitive->y1 =
        FIELD_REF(u16, g_battle_work, 0x30u) +
        ((func_801783C8(phase) * *radius) >> 12);
    next_i = i + 1;
    next_phase = next_i << 9;
    primitive->x2 =
        FIELD_REF(u16, g_battle_work, 0x2Eu) +
        ((func_801782FC(next_phase) * *radius) >> 12);
    i = next_i;
    primitive->y2 =
        FIELD_REF(u16, g_battle_work, 0x30u) +
        ((func_801783C8(next_phase) * *radius) >> 12);

    primitive->r0 = 0x80;
    primitive->g0 = 0x80;
    primitive->b0 = 0x80;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = 1;
    primitive->g2 = 1;
    primitive->b2 = 1;

    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0x1C);
  } while (i < 8);
}
