#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F74EC
 * @behavior Stamps the scratch work object's cursor trail toward the work-area
 * cell named by that object's cell index (byte 0x06): it copies the object's
 * cursor (x, y and scale word), starts the interpolated cursor at the object's
 * x with its y advanced by 0x8000, and keeps looping while the target cell's y
 * is not yet within that step, replacing the interpolated cursor's scale-word
 * high halfword with the func_8015477C distance of (x, y + 0x8000) each time;
 * once the target cell's y is reached it snaps that cursor's y and scale word
 * to the cell's own cursor and stops after one more stamp. Every iteration
 * emits one tinted sprite through func_801F78EC(interpolated cursor, object
 * cursor, tint), where the tint is the three signed channels of
 * g_WorkObjectTint, then advances the object cursor to the interpolated one.
 * When the shared mode byte D_80146866 reads 0x32 the object's state byte at
 * offset 0x01 is set to 2. Takes no arguments and returns nothing.
 * Clean-C shape: the loop-control flag is a 16-bit local (so the compare
 * extends it and the branch reads it out of $v0) and the loop tail copying
 * the interpolated cursor back into the object cursor is one 12-byte
 * aggregate assignment (so the last store stays ahead of that compare and the
 * 0x8000 step constant fills the branch delay slot).
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkObjectTrailToCell(void) {
  Scena00WorkCell* obj;
  u8* rec;
  Scena00Cursor cur;
  Scena00Cursor interp;
  Scena00Cursor target;
  Scena00Tint color;
  s16 more;

  obj = (Scena00WorkCell*)D_1F800044;
  color = *(Scena00Tint*)g_WorkObjectTint;
  rec = &D_80146888 + (u32)obj->index_06 * sizeof(Scena00WorkCell);
  cur.coord_x_00 = obj->cursor_34.coord_x_00;
  interp.coord_x_00 = cur.coord_x_00;
  cur.coord_y_04 = obj->cursor_34.coord_y_04;
  interp.coord_y_04 = cur.coord_y_04;
  cur.scale_08 = obj->cursor_34.scale_08;
  interp.scale_08 = cur.scale_08;
  target.coord_x_00 = *(s32*)(rec + 0x34);
  target.coord_y_04 = *(s32*)(rec + 0x38);
  target.scale_08.word = *(s32*)(rec + 0x3C);
  more = 1;
  do {
    interp.coord_x_00 = cur.coord_x_00;
    interp.coord_y_04 = cur.coord_y_04 + 0x8000;
    if (target.coord_y_04 < interp.coord_y_04) {
      interp.coord_y_04 = target.coord_y_04;
      interp.scale_08 = target.scale_08;
      more = 0;
    } else {
      interp.scale_08.parts.depth_02 =
          func_8015477C(cur.coord_x_00, interp.coord_y_04);
    }
    func_801F78EC(interp, cur, color);
    cur = interp;
  } while (more);

  if (D_80146866 == 50) {
    ((Scena00WorkCell*)D_1F800044)->state_01 = 2;
  }
}
