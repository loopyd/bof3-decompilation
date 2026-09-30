#include "bof3/world/area02613_internal.h"
#include "gpu/prim.h"

/* @source 0x801F2E04
 * @behavior emits one four-step ring of flat translucent triangles for the
 * passed work point: projects the point's three-word centre into the local
 * screen-point ring, then for each of the four ring angles advances the local
 * angle by 0x100 and rotates the passed radius through the shared angular
 * helpers over the point's centre, projecting that ring point as well; every
 * step writes a draw mode at the shared primitive cursor, links the cursor and
 * advances it by the passed 0x18-byte flat primitive through the shared
 * grid-cell linker and fills that flat triangle with the projected centre and
 * the previous and the new ring point in colour 0x80. The two projected points
 * live in the address-taken local ring whose 48 bytes reproduce the original
 * 0x80-byte frame; only its first two entries are read here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2E04(VECTOR* center, s32 scale, s32 start_angle, s32 xofs,
                   s32 yofs) {
  VECTOR                    point;
  World00Area026ScreenPoint screen[6];
  POLY_F3*                  prim;
  s32                       angle = start_angle;
  u8                        i;

  func_801AFF04(center, &screen[0]);

  point.vx = center->vx + ((scale * func_801783C8((u16)angle)) >> 12);
  point.vy = center->vy + ((scale * func_801782FC((u16)angle)) >> 12);
  point.vz = center->vz;
  func_801AFF04(&point, &screen[1]);

  i = 0;
  do {
    SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 2, 0x380, 0x100), 0);
    func_80155A08(center->vx + xofs, center->vy + yofs, 0, 0x18);

    prim = (POLY_F3*)g_PrimCursor;
    func_8017A954(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = screen[0].x;
    prim->y0 = screen[0].y;
    prim->x1 = screen[1].x;
    prim->y1 = screen[1].y;

    angle += 0x100;
    point.vx = center->vx + ((scale * func_801783C8((u16)angle)) >> 12);
    point.vy = center->vy + ((scale * func_801782FC((u16)angle)) >> 12);
    i++;
    func_801AFF04(&point, &screen[1]);

    prim->x2 = screen[1].x;
    prim->y2 = screen[1].y;
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;

    func_80155A08(center->vx + xofs, center->vy + yofs, 0, 0x18);
  } while (i < 4);
}
