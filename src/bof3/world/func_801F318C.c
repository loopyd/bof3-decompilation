#include "bof3/world/area02813_internal.h"

/* @source 0x801F318C
 * @behavior draws the 32-step rotation trail of the current AREA028 work
 *           record: seeds the trail with the record's own projected centre,
 *           then for each of the 32 ring angles builds the rotated point from
 *           the passed radius through the shared angular helpers, projects it,
 *           stores the pair into the trail and emits one translucent Gouraud
 *           triangle per step whose first vertex is the projected centre, whose
 *           second vertex is the previous ring point and whose third is the new
 *           one (its first colour white, the other two black).
 * @status partial
 * @match 98.77
 * @residual live audit: 161/163 instructions, 648 original bytes versus 652 current; body and 32-step loop byte-identical, sole residual one extra load-delay nop before the pre-loop trail entry's address materialisation.
 */
void func_801F318C(s16 arg0) {
  World00Area028ScreenPoint screen;
  World00Area028ScreenPoint current;
  VECTOR                    point;
  u8                        scratch[0x20];
  World00Area028ScreenPoint* trail;
  POLY_G3*                  prim;
  u16                       angle;
  s8                        i;

  SetDrawMode((DR_MODE*)WORLD00_AREA028_PRIMITIVE_PTR, 0, 1,
              GetTPage(0, 2, 0x3C0, 0), 0);
  func_8014E5A0(3, 0x0C);
  func_801AFE18(scratch);

  point.vx = D_1F800044->unk_34;
  point.vy = D_1F800044->unk_38;
  point.vz = D_1F800044->unk_3C;
  func_801AFF04(&point, &screen);
  WORLD00_AREA028_TRAIL[0] = screen;

  point.vx = D_1F800044->unk_34 + ((arg0 * func_801783C8(0xFF80)) >> 4);
  point.vy = D_1F800044->unk_38 + ((arg0 * func_801782FC(0xFF80)) >> 4);
  point.vz = D_1F800044->unk_3C;
  func_801AFF04(&point, &current);

  angle = 0xFF80;
  for (i = 0; i < 0x20; i++) {
    trail = WORLD00_AREA028_TRAIL;
    prim = (POLY_G3*)WORLD00_AREA028_PRIMITIVE_PTR;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = screen.x;
    prim->y0 = screen.y;
    angle = (u16)(angle + 0x80);
    prim->x1 = current.x;
    prim->y1 = current.y;

    point.vx = D_1F800044->unk_34 + ((arg0 * func_801783C8(angle)) >> 4);
    point.vy = D_1F800044->unk_38 + ((arg0 * func_801782FC(angle)) >> 4);
    point.vz = D_1F800044->unk_3C;
    func_801AFF04(&point, &current);

    prim->x2 = current.x;
    prim->y2 = current.y;
    trail[i + 1] = current;

    prim->r0 = 0xFF;
    prim->g0 = 0xFF;
    prim->b0 = 0xFF;
    prim->r1 = 0;
    prim->g1 = 0;
    prim->b1 = 0;
    prim->r2 = 0;
    prim->g2 = 0;
    prim->b2 = 0;

    func_8014E5A0(3, 0x1C);
  }
}
