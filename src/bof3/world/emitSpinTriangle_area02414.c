#include "bof3/world/area02414_internal.h"
#include "gpu/prim.h"

/* @source 0x801F3944
 * @behavior emits one translucent flat triangle for the spin-work record that
 * drawSpin passes: projects the record's own three-word point into the first
 * vertex, builds the second and third vertices from the record's
 * `+0x10`/`+0x12`/`+0x14` and `+0x18`/`+0x1a`/`+0x1c` halfwords rotated by the
 * `+0x24` angle, scaled by the `+0x28` factor and translated by the record's
 * `+0x00`/`+0x04`/`+0x08` words, then colours the first vertex from the
 * clamped `+0x2a` halfword (blue is half of red/green), zeroes the other two
 * vertices' colours and appends the 0x1c-byte primitive.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3944(World00Area024SpinWork* work) {
  POLY_G3* primitive;
  VECTOR   point;
  s16      alpha;
  u8       color;

  primitive = (POLY_G3*)g_PrimCursor;
  func_8017A97C(primitive);
  SetSemiTrans(primitive, 1);
  func_801AFF04(work, &primitive->x0);

  point.vx = ((func_801783C8(work->field_24) * work->field_10.vx -
               func_801782FC(work->field_24) * work->field_10.vy) >>
              8) *
             work->field_28;
  point.vy = ((func_801782FC(work->field_24) * work->field_10.vx +
               func_801783C8(work->field_24) * work->field_10.vy) >>
              8) *
             work->field_28;
  point.vz = (work->field_10.vz << 12) * work->field_28;
  point.vx += work->field_00;
  point.vy += work->field_04;
  point.vz += work->field_08;
  func_801AFF04(&point, &primitive->x1);

  point.vx = ((func_801783C8(work->field_24) * work->field_18.vx -
               func_801782FC(work->field_24) * work->field_18.vy) >>
              8) *
             work->field_28;
  point.vy = ((func_801782FC(work->field_24) * work->field_18.vx +
               func_801783C8(work->field_24) * work->field_18.vy) >>
              8) *
             work->field_28;
  point.vz = (work->field_18.vz << 12) * work->field_28;
  point.vx += work->field_00;
  point.vy += work->field_04;
  point.vz += work->field_08;
  func_801AFF04(&point, &primitive->x2);

  alpha = work->field_2a;
  if (alpha < 0) {
    color = 0;
  } else {
    color = alpha;
    if (alpha >= 0x100) {
      color = 0xFF;
    }
  }
  primitive->r0 = color;
  primitive->g0 = color;
  primitive->b0 = color >> 1;
  primitive->r1 = 0;
  primitive->g1 = 0;
  primitive->b1 = 0;
  primitive->r2 = 0;
  primitive->g2 = 0;
  primitive->b2 = 0;
  func_8014E5A0(1, 0x1C);
}
