#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F8360
 * @behavior Emits the scratchpad work object's entry-effect spin. The object's
 * fade countdown byte at offset 0x09 is converted from its 360-step unit into
 * the local 0x1000-per-turn angle unit as (byte << 12) / 0x168 and stored in
 * the object's word at offset 0x6C. The object's word at 0x10 (an angle in that
 * unit) and word at 0x0C (a radius) then build two three-vertex shapes, each
 * with its second vertex at (radius << 4, 0, 0) and its third vertex at
 * ((radius * func_801783C8(angle)) >> 8, (radius * func_801782FC(angle)) >> 8,
 * 0): the first shape uses the object's angle and the second the fixed angle
 * 0xAC. Twenty-four times (counter < 0x18) the function then re-reads the
 * primitive published at g_PrimCursor, stamps it through func_8017A97C inside a
 * PushMatrix/PopMatrix pair with the view matrix built by func_8015B410 and
 * func_8015B4B0, projects the shape selected by the counter's low bit (odd uses
 * the object's angle, even the fixed 0xAC) through RotTransPers3 into the
 * primitive's three vertex pairs at offsets 0x08/0x10/0x18, writes the object's
 * tint bytes at 0x5D/0x5E/0x5F into the primitive's colour bytes at
 * 0x04/0x05/0x06 and the negated doubled countdown byte into the primitive's
 * bytes at 0x0C/0x0D/0x0E and 0x14/0x15/0x16, marks the primitive
 * semi-transparent, appends it through func_8014E5A0(object byte 0x29, 0x1C)
 * and advances the object's word at 0x6C by 0xAA. Takes no arguments and
 * returns nothing.
 * Clean-C shapes retained: the two shapes' tip values are computed into locals
 * before the vertex stores (that statement order reproduces the original's
 * 152-byte frame, prologue, store schedule and 584-byte size), the tip products
 * are read as unsigned (the original shifts them logically while multiplying
 * signed), and the countdown byte that feeds the fade is read before the tint
 * bytes so one published-cell load serves both.
 * @status partial
 * @match 95.21
 * @residual Register-pair exchange at +0x0074: the original keeps radius << 4 in $s1 and the work-angle tip in $s2, this candidate holds them swapped, so the two tip definitions, the radius shift and their four vertex stores differ and all other 139 of 146 instructions match.
 * The residual is a callee-saved register-pair permutation only: the 584-byte
 * size, the 152-byte frame and every instruction schedule match. About 30
 * measured clean-C variants (statement order of the two shape groups, inline
 * versus local tip values, declaration order, s32/u32/u16/s16/long tip types,
 * (s16)/(u16) store casts, pointer hoists for the second vertex) keep this
 * identical swap or regress. The compiler profile probe (bin/flag-search) and
 * the permuter are the next untried rungs and are opt-in for this selector.
 */
void func_801F8360(void) {
  MATRIX   matrix;
  SVECTOR  shape_work[3];
  SVECTOR  shape_fixed[3];
  POLY_G3* prim;
  long     depth;
  long     flag;
  s32      radius;
  s32      angle;
  s32      fade;
  s16      index;
  s32      tip_work_x;
  s32      tip_work_y;
  s32      tip_fixed_x;
  s32      tip_fixed_y;

  *(s32*)(D_1F800044 + 0x6C) = (D_1F800044[0x09] << 12) / 0x168;

  angle = *(s32*)(D_1F800044 + 0x10);
  radius = *(s32*)(D_1F800044 + 0x0C);

  tip_work_x = (u32)(radius * func_801783C8(angle)) >> 8;
  tip_work_y = (u32)(radius * func_801782FC(angle)) >> 8;

  shape_work[0].vx = 0;
  shape_work[0].vy = 0;
  shape_work[0].vz = 0;
  shape_work[1].vx = radius << 4;
  shape_work[1].vy = 0;
  shape_work[1].vz = 0;
  shape_work[2].vx = tip_work_x;
  shape_work[2].vy = tip_work_y;
  shape_work[2].vz = 0;

  tip_fixed_x = (u32)(radius * func_801783C8(0xAC)) >> 8;
  tip_fixed_y = (u32)(radius * func_801782FC(0xAC)) >> 8;

  shape_fixed[0].vx = 0;
  shape_fixed[0].vy = 0;
  shape_fixed[0].vz = 0;
  shape_fixed[1].vx = radius << 4;
  shape_fixed[1].vy = 0;
  shape_fixed[1].vz = 0;
  shape_fixed[2].vx = tip_fixed_x;
  shape_fixed[2].vy = tip_fixed_y;
  shape_fixed[2].vz = 0;

  for (index = 0; index < 0x18; index++) {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    PushMatrix();
    func_8015B410(&matrix);
    func_8015B4B0(&matrix);

    if (index & 1) {
      RotTransPers3(&shape_work[0], &shape_work[1], &shape_work[2],
                    (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                    &depth, &flag);
    } else {
      RotTransPers3(&shape_fixed[0], &shape_fixed[1], &shape_fixed[2],
                    (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                    &depth, &flag);
    }

    fade = -(D_1F800044[0x09] << 1);
    prim->r0 = D_1F800044[0x5D];
    prim->g0 = D_1F800044[0x5E];
    prim->b0 = D_1F800044[0x5F];
    prim->r1 = fade;
    prim->g1 = fade;
    prim->b1 = fade;
    prim->r2 = fade;
    prim->g2 = fade;
    prim->b2 = fade;

    SetSemiTrans(prim, 1);
    func_8014E5A0(D_1F800044[0x29], 0x1C);
    PopMatrix();

    *(s32*)(D_1F800044 + 0x6C) += 0xAA;
  }
}
