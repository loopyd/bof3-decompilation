#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */
extern s32 GetGraphType(void);

/* The loop uses the triangle and phase-expression shapes of magic097/03 at
 * 0x801F083C, with this span's radius, packet placement and colour stores.
 * A payload cmp proved that magic091/03 has the same 564-byte original span
 * at offset 0xBB0; its boundary was asm when this source was authored.
 */
/* @source 0x801EF7B0
 * @behavior Draw eight translucent Gouraud fan triangles around work halfwords
 *           0x2E and 0x30. Store twice unsigned work byte 9 plus 32 in scratchpad
 *           halfword 0x1F800000 and use its signed value as the radius. Each
 *           triangle joins the centre to consecutive phase samples spaced by
 *           0x200. Centre red is ten times work byte 9; all other colour bytes
 *           are one. Each iteration emits a draw-mode packet with tpage 0xA5
 *           for graph type 1 or 2, otherwise 0x35, then the triangle. Link the
 *           0xC-byte and 0x1C-byte packets through 0x80155A08 with work words
 *           0x34 and 0x38, kind 2. The dispatcher at 0x801EEC10 calls this at
 *           0x801EEC9C without arguments and consumes no return value.
 * @status partial
 * @match 82.98
 * @residual 117/141 instructions; 560/564 bytes. Entry work-cell load is
 *           hoisted before the frame, counter and constant-one registers differ,
 *           and the radius base is formed directly rather than copied.
 */
void drawTranslucentGouraudFanOfEightTrianglesAtWorkScreenPairRadiusTwiceWorkByte9Plus32TintedByWorkByte9TimesTenBmagicMagic05503_801EF7B0(void)
{
  POLY_G3 *primitive;
  s16 *radius;
  s32 phase;
  s32 tpage;
  s32 i;
  s32 tint;

  i = 0;
  radius = &D_1F800000;
  *radius = D_1F800044[9] * 2 + 32;
  do {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0xA5;
    } else {
      tpage = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x34u),
                  FIELD_REF(s32, D_1F800044, 0x38u), 2, 0xC);
    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = FIELD_REF(u16, D_1F800044, 0x2Eu);
    primitive->y0 = FIELD_REF(u16, D_1F800044, 0x30u);
    phase = i << 9;
    primitive->x1 = FIELD_REF(u16, D_1F800044, 0x2Eu) +
        ((func_801782FC(phase) * *radius) >> 12);
    primitive->y1 = FIELD_REF(u16, D_1F800044, 0x30u) +
        ((func_801783C8(phase) * *radius) >> 12);
    i++;
    phase = i << 9;
    primitive->x2 = FIELD_REF(u16, D_1F800044, 0x2Eu) +
        ((func_801782FC(phase) * *radius) >> 12);
    primitive->y2 = FIELD_REF(u16, D_1F800044, 0x30u) +
        ((func_801783C8(phase) * *radius) >> 12);
    tint = D_1F800044[9];
    primitive->g0 = 1;
    primitive->b0 = 1;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = 1;
    primitive->g2 = 1;
    primitive->b2 = 1;
    primitive->r0 = tint * 10;
    func_80155A08(FIELD_REF(s32, D_1F800044, 0x34u),
                  FIELD_REF(s32, D_1F800044, 0x38u), 2, 0x1C);
  } while (i < 8);
}
