#include "bof3/scenario/scena00_internal.h"

extern u16 D_801FE9EC[];
s32 func_801786E8(s32 arg0);
void func_8017A9E0(void* arg0);

/* @source 0x801F823C
 * @behavior Fills a screen-aligned circle of radius arg2 with translucent
 * Gouraud-tinted textured strips: for each of the 2*arg2 rows it emits two
 * 0x34-byte POLY_GT4 quads, one for each half of the strip, so the pair covers
 * the row from the left rim to the right rim in one-pixel-tall bands. The row's
 * y is arg1 - arg2 + i, and its half width is the half chord
 * func_801786E8(arg2*arg2 - m*m) with m counting down from arg2 - 1, the
 * overlay-local square-root helper over the circle's own squared row offset;
 * that half chord also shifts with the rotating angle the overlay keeps in the
 * unsigned-halfword table at 0x801FE9EC, indexed by the row counter's low byte
 * and read through the local sine helper func_801782FC as
 * (arg3 * sine) >> 12, the same twelve-bit fixed-point scale the overlay's other
 * emitters apply. The left quad spans arg0 - half + offset to arg0 + offset and
 * the right quad arg0 + offset to arg0 + offset + half, where offset is that
 * scaled sine, so both quads share their inner edge. Both take the texture page
 * GetTPage(2, 0, 0x340, 0x100), are marked non-translucent through SetSemiTrans
 * with zero, are stamped through the cursor setter func_8017A9E0 and are
 * appended through func_8014E5A0 under allocation class 2. The left quad's
 * texture column runs from 0x40 - half to 0x40 and the right quad's from 0 to
 * half, so the 0x40-wide texture's centre sits on the shared inner edge; the row
 * index i - (arg2 - 0x40) is the texture row and the right quad samples it 0x80
 * higher, the second half of a 0x100-row texture. The left quad's first and
 * third vertices carry the blue-ish tint 0x40/0x40/0x80 and its second and
 * fourth the grey 0x60/0x60/0x60; the right quad's first and third carry that
 * grey and its second and fourth the red-ish 0x80/0x40/0x40, so each row fades
 * between the two rims. The cursor pointer is re-read for the second quad.
 * Callers: the overlay's per-frame drawing step at 0x801F8534 is the only
 * referral and passes the scratchpad work object's signed halfwords 0x2E, 0x30,
 * 0x32 and 0x2C; the argument roles above are read from those four halfwords and
 * from this body, which reads each once per row. The 0x78-byte frame keeps $ra,
 * $fp as the row counter and $s0-$s7 across the calls, and the four arguments,
 * the row counter and its two derived words sit in the frame's local halfword
 * slots.
 * @status partial
 * @match 74.74
 * @residual Allocation and scheduling only: the body, call order, constants,
 * frame size and every local slot agree with the original, and 142 of its 190
 * instructions match. The original keeps the row value in $s0 and the
 * texture-row value in $s1 where this candidate has them exchanged across
 * $s0/$s1, its prologue places the loop guard and the four argument stores in a
 * different order, and its twelve colour stores come in a different order; the
 * extra move this candidate once emitted for the row-plus-one value is gone.
 * Probes that rewrote the texture-row expression as one or two forms, added or
 * removed the named row-plus-one and half-chord locals, reordered the argument
 * declarations and stores, and used initializer declarations all measured at or
 * below this score.
 */
void drawRotatingTexturedGouraudCircleStripsAtScreenPairFromAngleTableScenarioScena0800_801F823C(
    s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
  POLY_GT4* prim;
  u16 x;
  u16 y;
  u16 n;
  u16 r;
  u16 m;
  u16 row;
  u16 left;
  u16 next;
  u16 right;
  s32 bound;
  s32 nn;
  s32 half;
  s32 sine;
  s32 offset;
  u8 nb;
  u8 i;

  x = arg0;
  y = arg1;
  n = arg2;
  r = arg3;
  nn = (s16)n;
  bound = nn * 2;
  m = n - 1;
  nb = n;
  for (i = 0; i < bound; i++) {
    half = func_801786E8(nn * nn - (s16)m * (s16)m);
    sine = func_801782FC(D_801FE9EC[i]);
    m--;
    offset = ((s16)r * sine) >> 12;
    prim = (POLY_GT4*)g_PrimCursor;
    func_8017A9E0(prim);
    SetSemiTrans(prim, 0);
    row = y - n + i;
    next = row + 1;
    left = x - half + offset;
    right = left + half;
    prim->x0 = left;
    prim->y0 = row;
    prim->x1 = right;
    prim->y1 = row;
    prim->x2 = left;
    prim->y2 = next;
    prim->x3 = right;
    prim->y3 = next;
    prim->u0 = 0x40 - half;
    prim->v0 = i - (nb - 0x40);
    prim->u1 = 0x40;
    prim->v1 = i - (nb - 0x40);
    prim->u2 = 0x40 - half;
    prim->v2 = i - (nb - 0x40) + 1;
    prim->u3 = 0x40;
    prim->v3 = i - (nb - 0x40) + 1;
    prim->tpage = GetTPage(2, 0, 0x340, 0x100);
    prim->r0 = 0x40;
    prim->g0 = 0x40;
    prim->b0 = 0x80;
    prim->r1 = 0x60;
    prim->g1 = 0x60;
    prim->b1 = 0x60;
    prim->r2 = 0x40;
    prim->g2 = 0x40;
    prim->b2 = 0x80;
    prim->r3 = 0x60;
    prim->g3 = 0x60;
    prim->b3 = 0x60;
    func_8014E5A0(2, 0x34);
    prim = (POLY_GT4*)g_PrimCursor;
    func_8017A9E0(prim);
    SetSemiTrans(prim, 0);
    prim->x0 = x + offset;
    prim->y0 = row;
    prim->x1 = x + offset + half;
    prim->y1 = row;
    prim->x2 = x + offset;
    prim->y2 = next;
    prim->x3 = x + offset + half;
    prim->y3 = next;
    prim->u0 = 0;
    prim->v0 = i - (nb - 0x40) - 0x80;
    prim->u1 = half;
    prim->v1 = i - (nb - 0x40) - 0x80;
    prim->u2 = 0;
    prim->v2 = i - (nb - 0x40) - 0x7F;
    prim->u3 = half;
    prim->v3 = i - (nb - 0x40) - 0x7F;
    prim->tpage = GetTPage(2, 0, 0x340, 0x100);
    prim->r0 = 0x60;
    prim->g0 = 0x60;
    prim->b0 = 0x60;
    prim->r1 = 0x80;
    prim->g1 = 0x40;
    prim->b1 = 0x40;
    prim->r2 = 0x60;
    prim->g2 = 0x60;
    prim->b2 = 0x60;
    prim->r3 = 0x80;
    prim->g3 = 0x40;
    prim->b3 = 0x40;
    func_8014E5A0(2, 0x34);
  }
}
