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
 * @status partial
 * @match 95.88
 * @residual live asm-diff 93/97 insns (388->380 bytes), first difference +0x013c: the loop-back branch. The original materialises the loop-control flag into $v0 (`move v0,s0`) before `bnez v0,<loop head>` and re-loads the 0x8000 step constant in that branch's delay slot, so its loop head is the first load; this shape branches on $s0 directly and keeps the step constant at the loop head. Declaring the control flag as `s16` (or casting the condition) does make the compiler emit the `move`, which suggests a 16-bit control flag, but the copy is then scheduled between the loop's tail stores, so the delay slot still takes the last store instead of the step constant. Ranked untried experiments: (1) a `s16` flag with an explicit late condition copy (a temp assigned after the tail stores), (2) the same loop with the tail cursor copy written as a 12-byte aggregate assignment, (3) a per-object compiler/scheduling profile probe (bin/flag-search; out of scope for this mission). All other 93 instructions, the whole prologue/init/record copy/argument image and the other delay-slot pairs match the original byte-for-byte.
 */
void drawWorkObjectTrailToCell(void) {
  Scena00WorkCell* obj;
  u8* rec;
  Scena00Cursor cur;
  Scena00Cursor interp;
  Scena00Cursor target;
  Scena00Tint color;
  s32 more;

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
    cur.coord_x_00 = interp.coord_x_00;
    cur.coord_y_04 = interp.coord_y_04;
    cur.scale_08 = interp.scale_08;
  } while (more);

  if (D_80146866 == 50) {
    ((Scena00WorkCell*)D_1F800044)->state_01 = 2;
  }
}
