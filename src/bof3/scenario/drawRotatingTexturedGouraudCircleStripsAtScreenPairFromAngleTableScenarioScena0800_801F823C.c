#include "bof3/scenario/scena00_internal.h"

extern u16 D_801FE9EC[];
s32 func_801786E8(s32 arg0);
void func_8017A9E0(void* arg0);

/* @source 0x801F823C
 * @behavior Narrows the four inputs to halfwords and emits two textured
 * Gouraud quads per row of a circle. The signed radius is the low halfword of
 * arg2; a nonpositive radius returns immediately. The byte row counter is
 * compared with twice that radius, so radii 1..127 finish after twice the
 * radius in rows, while radii 128..32767 cannot satisfy the exit test.
 * For each row, func_801786E8 supplies the half chord from the squared radius
 * and descending halfword row offset. A halfword table entry at 0x801FE9EC
 * feeds func_801782FC; multiplying its answer by the signed low halfword of
 * arg3 and shifting by 12 supplies the horizontal displacement. The quads
 * share their inner edge and cover the left and right halves of the row.
 * Both use GetTPage(2, 0, 0x340, 0x100), SetSemiTrans with zero and allocation
 * class 2 with size 0x34. The left texture columns run from 0x40 minus the
 * half chord to 0x40; the right columns run from zero to the half chord, with
 * texture rows displaced by 0x80. The left quad fades blue to grey and the
 * right fades grey to red. The primitive cursor is sampled separately for
 * each quad. The caller at 0x801F8534 supplies scratchpad halfwords 0x2E,
 * 0x30, 0x32 and 0x2C. The original 0x78-byte frame and ten calls per row are
 * preserved.
 * @status partial
 * @match 88.95
 * @residual 169/190 instructions at the original 760 bytes; first difference
 *   +0x00D0. Counter/cursor order, first-quad geometry and UV calculations,
 *   and counter-increment placement differ. Saved register roles, both
 *   colour sequences and the final loop-test sequence match.
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
  u8 half_uv;
  u8 nb;
  u8 i;

  x = arg0;
  y = arg1;
  n = arg2;
  r = arg3;
  m = n - 1;
  i = 0;
  if ((s16)n * 2 > 0) {
    nn = (s16)n;
    bound = nn * 2;
    nb = n;
    do {
    half = func_801786E8(nn * nn - (s16)m * (s16)m);
    sine = func_801782FC(D_801FE9EC[i]);
    do {
    do {
    offset = ((s16)r * sine) >> 12;
    prim = (POLY_GT4*)g_PrimCursor;
    m--;
    func_8017A9E0(prim);
    do {
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
    prim->r0 = prim->g0 = 0x40;
    prim->b0 = 0x80;
    prim->r1 = prim->g1 = prim->b1 = 0x60;
    prim->r2 = prim->g2 = 0x40;
    prim->b2 = 0x80;
    prim->r3 = prim->g3 = prim->b3 = 0x60;
    func_8014E5A0(2, 0x34);
    } while (0);
    } while (0);
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
    half_uv = (u8)half;
    prim->u0 = 0;
    prim->v0 = i - (nb - 0x40) - 0x80;
    prim->u1 = half_uv;
    prim->v1 = i - (nb - 0x40) - 0x80;
    prim->u2 = 0;
    prim->v2 = i - (nb - 0x40) - 0x7F;
    prim->u3 = half_uv;
    prim->v3 = i - (nb - 0x40) - 0x7F;
    prim->tpage = GetTPage(2, 0, 0x340, 0x100);
    prim->r0 = prim->g0 = prim->b0 = 0x60;
    prim->r1 = 0x80;
    prim->g1 = prim->b1 = 0x40;
    prim->r2 = prim->g2 = prim->b2 = 0x60;
    prim->r3 = 0x80;
    prim->g3 = prim->b3 = 0x40;
    func_8014E5A0(2, 0x34);
    } while (0);
    i++;
    } while (i < bound);
  }
}
