#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7670
 * @behavior Stamps two tinted cursor trails from the scratch work object toward
 * the work-area cell named by that object's cell index at byte 0x06, then
 * advances the object's state byte at 0x01 to 3 once the shared mode byte
 * D_80146866 reads 0x34. The first pass copies the object's cursor (x, y and
 * scale word) and the shared tint g_WorkObjectTint, starts the interpolated
 * cursor at the object's x with its y advanced by 0x8000 and keeps looping
 * while the target cell's y is not yet within that step, replacing the
 * interpolated cursor's scale-word high halfword with the func_8015477C
 * distance of (x, y + 0x8000); once the target cell's y is reached it snaps
 * that cursor's y and scale word to the cell's own cursor and stops one stamp
 * later. Each iteration emits one tinted sprite through
 * func_801F78EC(interpolated cursor, object cursor, tint) and then advances the
 * object cursor to the interpolated one. The second pass reseats the object
 * cursor at the fixed position 0xB0000/0x138000 with its scale-word high
 * halfword set to the func_8015477C distance of that pair, switches the tint to
 * the fixed 0x30/0x30/0x30 and walks the interpolated cursor the other way (y
 * minus 0x8000) toward the same cell cursor, snapping to the cell cursor as
 * soon as the interpolated y drops below it, again emitting one sprite per
 * iteration. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The frame slot below the cursors is a reconstruction of a declaration the
 * original translation unit carried but this overlay's emitted code never
 * references: the original frame is 0x68 with vars=64 while this function's
 * locals need vars=56, it saves only $s0/$ra, and no instruction in the
 * original body reads or writes the 8 bytes at 0x20-0x27 (all sp offsets are
 * 0x10/0x14/0x18-0x1A, 0x28-0x32, 0x38-0x42, 0x48-0x50, 0x58-0x5A, 0x60,
 * 0x64). Declaring one small aggregate restores that reservation exactly; the
 * name and type are a reconstruction choice constrained only by the slot size
 * (a 3-byte Scena00Tint rounds to the 8-byte slot the original reserves).
 * Flagged for independent review; removing the declaration leaves only the
 * frame-size/offset residual.
 */
void func_801F7670(void) {
  Scena00Tint unused_frame_slot;
  Scena00WorkCell* obj;
  u8* rec;
  Scena00Cursor cur;
  Scena00Cursor interp;
  Scena00Cursor target;
  Scena00Tint color;
  s16 more;

  (void)unused_frame_slot;

  obj = (Scena00WorkCell*)D_1F800044;
  color = *(Scena00Tint*)g_WorkObjectTint;
  rec = &D_80146888 + (u32)obj->index_06 * sizeof(Scena00WorkCell);
  cur.coord_x_00 = obj->cursor_34.coord_x_00;
  cur.coord_y_04 = obj->cursor_34.coord_y_04;
  cur.scale_08 = obj->cursor_34.scale_08;
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

  cur.coord_x_00 = 0xB0000;
  cur.coord_y_04 = 0x138000;
  cur.scale_08.parts.depth_02 = func_8015477C(0xB0000, 0x138000);
  color.r = 0x30;
  color.g = 0x30;
  color.b = 0x30;

  more = 1;
  do {
    interp.coord_x_00 = cur.coord_x_00;
    interp.coord_y_04 = cur.coord_y_04 - 0x8000;
    if (interp.coord_y_04 < target.coord_y_04) {
      interp.coord_y_04 = target.coord_y_04;
      interp.scale_08 = target.scale_08;
      more = 0;
    } else {
      interp.scale_08.parts.depth_02 =
          func_8015477C(cur.coord_x_00, interp.coord_y_04);
    }
    func_801F78EC(cur, interp, color);
    cur = interp;
  } while (more);

  if (D_80146866 == 0x34) {
    ((Scena00WorkCell*)D_1F800044)->state_01 = 3;
  }
}
