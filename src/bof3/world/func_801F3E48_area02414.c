#include "bof3/world/area02414_internal.h"
#include "gpu/prim.h"

/* @source 0x801F3E48
 * @behavior draws the closed flat-line quads of the overlay's quad record list:
 *           for each of the leading signed count byte's 0x28-byte records it
 *           repoints the scratch cursor at D_80147A58 inside a PushMatrix/
 *           PopMatrix pair, builds the view matrix through func_8015B410,
 *           copies it for func_8015B4B0, sets the semi-transparent line draw
 *           mode, then per record emits a LINE_F4 whose three colour bytes are
 *           the colour argument, projects the record's four packed corners
 *           through RotTransPers and appends the 0x1c-byte primitive through
 *           func_8014E5A0(1, 0x1c) only when either of the quad's two
 *           triangles survives the func_801F4158 cross-product test.
 * @status exact
 * @match 100.00
 * @residual none
 * The count read and the counter initializer straddle the func_8015B410 call;
 * that statement order is what reproduces the original's addiu s0 / move s7
 * scheduling at the top of the block.
 */
s32 func_801F3E48(u8 color) {
  SVECTOR  corners[4];
  VECTOR   origin;
  MATRIX   matrix;
  MATRIX   matrixCopy;
  long     depth;
  long     flag;
  u16      count;
  u8*      saved;
  u16*     record;
  LINE_F4* primitive;
  u16      index;

  origin = D_801F2C04;

  saved = D_1F800044;
  D_1F800044 = (u8*)&D_80147A58;

  PushMatrix();

  count = (s8)WORLD00_AREA024_RECORD_LIST[0];
  func_8015B410(&matrix);
  index = 0u;
  SetRotMatrix(&matrix);
  SetTransMatrix(&matrix);

  matrixCopy = matrix;
  func_8015B4B0(&matrixCopy);

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x380, 0x100), 0);
  func_8014E5A0(1u, 0x0cu);

  record = (u16*)WORLD00_AREA024_SOURCE_TABLE;

  while (index < count) {
    primitive = (LINE_F4*)g_PrimCursor;

    record += 1;
    SetLineF4(primitive);
    SetSemiTrans(primitive, 1);
    primitive->r0 = color;
    primitive->g0 = color;
    primitive->b0 = color;

    corners[0].vx = *record++;
    corners[0].vy = *record++;
    corners[0].vz = *record++;
    corners[1].vx = *record++;
    corners[1].vy = *record++;
    corners[1].vz = *record++;
    corners[2].vx = *record++;
    corners[2].vy = *record++;
    corners[2].vz = *record++;
    corners[3].vx = *record++;
    corners[3].vy = *record++;
    corners[3].vz = *record++;
    record += 7;

    RotTransPers(&corners[0], (long*)&primitive->x0, &depth, &flag);
    RotTransPers(&corners[1], (long*)&primitive->x1, &depth, &flag);
    RotTransPers(&corners[2], (long*)&primitive->x3, &depth, &flag);
    RotTransPers(&corners[3], (long*)&primitive->x2, &depth, &flag);

    if (func_801F4158(&primitive->x0, &primitive->x1, &primitive->x2) > 0 ||
        func_801F4158(&primitive->x2, &primitive->x3, &primitive->x0) > 0) {
      func_8014E5A0(1u, 0x1cu);
    }

    index += 1u;
  }

  PopMatrix();
  D_1F800044 = saved;
}
