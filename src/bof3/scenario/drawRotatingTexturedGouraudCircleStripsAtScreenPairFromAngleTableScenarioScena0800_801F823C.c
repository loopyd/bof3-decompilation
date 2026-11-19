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
 * @match 98.42
 * @residual 187/190 instructions at the original 760 bytes; first difference
 *   +0x00D0. Countdown/cursor loads and the decrement/argument move differ
 *   in order. The right upper UV offset uses +128 instead of -128.
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
  s32 bound;
  s32 more;
  s32 nn;
  s32 half;
  s32 sine;
  s32 offset;
  u8 top;
  u8 texture_row;
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
    prim->x0 = x - half + offset;
    prim->y0 = y - n + i;
    prim->x1 = (u16)(x - half + offset) + half;
    prim->y1 = y - n + i;
    prim->y2 = (y - n + i) + 1;
    prim->x3 = (u16)(x - half + offset) + half;
    prim->y3 = (y - n + i) + 1;
    row = y - n + i;
    texture_row = i + 0x40u;
    prim->x2 = x - half + offset;
    prim->u0 = 0x40 - half;
    top = nb;
    texture_row -= top;
    top -= 0x40u;
    prim->v0 = i - top;
    prim->u1 = 0x40;
    prim->v1 = i - top;
    prim->u2 = 0x40 - half;
    prim->v2 = texture_row + 1;
    prim->u3 = 0x40;
    prim->v3 = texture_row + 1;
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
    i++;
    func_8017A9E0(prim);
    SetSemiTrans(prim, 0);
    prim->x0 = x + offset;
    prim->y0 = row;
    prim->x1 = x + offset + half;
    prim->y1 = row;
    prim->x2 = x + offset;
    prim->y2 = row + 1;
    prim->x3 = x + offset + half;
    half_uv = (u8)half;
    prim->v0 = texture_row - 0x80;
    prim->u1 = half_uv;
    prim->v1 = texture_row - 0x80;
    prim->v2 = texture_row - 0x7F;
    prim->u3 = half_uv;
    prim->v3 = texture_row - 0x7F;
    prim->y3 = row + 1;
    prim->u0 = 0;
    prim->u2 = 0;
    prim->tpage = GetTPage(2, 0, 0x340, 0x100);
    prim->r0 = prim->g0 = prim->b0 = 0x60;
    prim->r1 = 0x80;
    prim->g1 = prim->b1 = 0x40;
    prim->r2 = prim->g2 = prim->b2 = 0x60;
    prim->r3 = 0x80;
    prim->g3 = prim->b3 = 0x40;
    func_8014E5A0(2, 0x34);
    more = i < bound;
    } while (0);
    } while (more);
  }
}
