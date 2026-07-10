#include "internal.h"

/* does: projects one radial step pair, then emits four translucent `POLY_F3`
 * slices around the source point using the supplied local offsets.
 * @source: 0x801f2e04 FUN_801f2e04
 */
void func_801f2e04(const s32* arg0, s32 arg1, s16 arg2, s32 arg3, s32 arg4) {
  s32      point[3];
  u16      screen_a[2];
  u16      screen_b[2];
  POLY_F3* primitive;
  u8       i;
  u8       color;

  func_801aff04(arg0, screen_a);

  point[0] = arg0[0] + ((arg1 * rcos(arg2)) >> 12);
  point[1] = arg0[1] + ((arg1 * rsin(arg2)) >> 12);
  point[2] = arg0[2];
  func_801aff04(point, screen_b);

  i = 0u;
  color = 0x80u;
  do {
    SetDrawMode((DR_MODE*)WORLD00_AREA026_13_PRIMITIVE_PTR, 0, 0,
                GetTPage(0, 2, 0x380, 0x100), NULL);
    func_80155a08(arg0[0] + arg3, arg0[1] + arg4, 0, 0x18);

    primitive = (POLY_F3*)WORLD00_AREA026_13_PRIMITIVE_PTR;
    SetPolyF3(primitive);
    SetSemiTrans(primitive, 1);

    primitive->x0 = screen_a[0];
    arg2 += 0x100;
    primitive->y0 = screen_a[1];
    primitive->x1 = screen_b[0];
    primitive->y1 = screen_b[1];

    point[0] = arg0[0] + ((arg1 * rcos(arg2)) >> 12);
    point[1] = arg0[1] + ((arg1 * rsin(arg2)) >> 12);
    i += 1u;
    func_801aff04(point, screen_b);

    primitive->x2 = screen_b[0];
    primitive->r0 = color;
    primitive->g0 = color;
    primitive->b0 = color;
    primitive->y2 = screen_b[1];

    func_80155a08(arg0[0] + arg3, arg0[1] + arg4, 0, 0x18);
  } while (i < 4u);
}
