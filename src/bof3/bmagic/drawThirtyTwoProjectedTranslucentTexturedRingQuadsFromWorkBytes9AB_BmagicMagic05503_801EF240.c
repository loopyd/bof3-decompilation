#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s16 D_1F800006; /* @source 0x1F800006 @kind data */
extern s16 D_1F800008; /* @source 0x1F800008 @kind data */
extern SVECTOR D_1F80001C; /* @source 0x1F80001C @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F800020; /* @source 0x1F800020 @kind data */
extern s16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern s16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern s16 D_1F800030; /* @source 0x1F800030 @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */
extern void func_8017A9E0(POLY_GT4 *primitive); /* @source 0x8017A9E0 */
extern s32 GetGraphType(void);

/* @source 0x801EF240
 * @behavior Draw 32 semi-transparent textured Gouraud quads joining consecutive
 *           ring samples at phases spaced by 0x80. Publish work byte 9 times
 *           four and times three, (work byte A plus 16) times four and the low
 *           four bits of work byte B times 256 into scratchpad halfwords.
 *           The first two scales drive the two rings; the third bends the
 *           first ring's z by the phase sampler minus its scale. Add the first
 *           ring x times 512 to both work words 0x34 and 0x38 for packet linking.
 *           Emit a draw-mode packet and a 0x34-byte POLY_GT4 per step, with
 *           tpage GetTPage(0,2,0x340,0x100), clut GetClut(0,0x1FA), texture u
 *           advancing four per step and texture v based on work byte B. Tint
 *           the first edge grey 1 and the other edge 104 minus three times
 *           work byte B. Project the four scratchpad vectors with RotTransPers4.
 *           The call at 0x801EEFDC supplies no arguments and uses no result.
 * @status partial
 * @match 83.28
 * @residual 254/304 instructions; 1220/1216 bytes. First difference is the
 *           zero argument setup around the first vector-base materialisation.
 *           Preheader initializers, vector-copy scheduling and the loop latch
 *           also differ.
 */
void drawThirtyTwoProjectedTranslucentTexturedRingQuadsFromWorkBytes9AB_BmagicMagic05503_801EF240(void)
{
  u8 *work;
  s16 *radius;
  SVECTOR *vectors;
  s16 *phasecell;
  u16 *shade;
  POLY_GT4 *primitive;
  s32 i;
  s32 u;
  s32 phase;
  s32 tpage;
  s32 x;
  s32 y;
  long p;
  long flag;

  work = D_1F800044;
  radius = &D_1F800000;
  *radius = work[9] << 2;
  D_1F800004 = work[9] * 3;
  D_1F800006 = (work[0xA] + 0x10) << 2;
  D_1F800008 = (work[0xB] & 0xF) << 8;
  vectors = &D_1F80001C;
  vectors->vx = (func_801783C8(0) * *radius) >> 12;
  D_1F80001E = (func_801782FC(0) * *radius) >> 12;
  D_1F800020 = ((func_801782FC(D_1F800008) * D_1F800006) >> 12) - D_1F800006;
  D_1F80002C = (func_801783C8(0) * D_1F800004) >> 12;
  D_1F80002E = (func_801782FC(0) * D_1F800004) >> 12;
  D_1F800030 = 0;
  phasecell = radius + 4;
  vectors--;
  shade = (u16 *)(radius + 5);
  u = 8;
  i = 1;
  do {
    phase = i << 7;
    vectors[0].vx = vectors[1].vx;
    vectors[0].vy = vectors[1].vy;
    vectors[0].vz = vectors[1].vz;
    *phasecell = ((i + D_1F800044[0xB]) & 0xF) << 8;
    vectors[1].vx = (func_801783C8(phase) * phasecell[-4]) >> 12;
    vectors[1].vy = (func_801782FC(phase) * phasecell[-4]) >> 12;
    vectors[1].vz = ((func_801782FC(*phasecell) * phasecell[-1]) >> 12) - phasecell[-1];
    vectors[2].vz = 0;
    vectors[2].vy = vectors[3].vy;
    vectors[2].vx = vectors[3].vx;
    vectors[3].vx = (func_801783C8(phase) * phasecell[-2]) >> 12;
    vectors[3].vy = (func_801782FC(phase) * phasecell[-2]) >> 12;
    vectors[3].vz = 0;
    work = D_1F800044;
    x = (vectors[1].vx << 9) + FIELD_REF(s32, work, 0x34u);
    y = (vectors[1].vx << 9) + FIELD_REF(s32, work, 0x38u);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      tpage = 0x125;
    } else {
      tpage = 0x55;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
    func_80155A08(x, y, 2, 0xC);
    primitive = (POLY_GT4 *)g_PrimCursor;
    func_8017A9E0(primitive);
    SetSemiTrans(primitive, 1);
    primitive->tpage = GetTPage(0, 2, 0x340, 0x100);
    primitive->clut = GetClut(0, 0x1FA);
    primitive->u0 = i << 2;
    primitive->v0 = (D_1F800044[0xB] << 2) + 0x20;
    primitive->u1 = u;
    primitive->v1 = (D_1F800044[0xB] << 2) + 0x20;
    primitive->u2 = i << 2;
    primitive->v2 = (D_1F800044[0xB] << 2) + 0x30;
    primitive->u3 = u;
    primitive->v3 = (D_1F800044[0xB] << 2) + 0x30;
    *shade = 0x68 - D_1F800044[0xB] * 3;
    primitive->r0 = 1;
    primitive->g0 = 1;
    primitive->b0 = 1;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = *shade;
    primitive->g2 = *shade;
    primitive->b2 = *shade;
    primitive->r3 = *shade;
    primitive->g3 = *shade;
    primitive->b3 = *shade;
    i++;
    u += 4;
    RotTransPers4(vectors, vectors + 1, vectors + 2, vectors + 3,
                  (long *)&primitive->x0, (long *)&primitive->x1,
                  (long *)&primitive->x2, (long *)&primitive->x3, &p, &flag);
    func_80155A08(x, y, 2, 0x34);
  } while (i < 0x21);
}
