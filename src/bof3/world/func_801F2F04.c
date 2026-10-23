#include "bof3/world/area03213_internal.h"

/* @source 0x801F2F04
 * @behavior Emits 32 semi-transparent Gouraud triangles as a radial fan.
 * Each triangle joins the center to two radius-scaled angular samples, with
 * the next sample wrapping to angle zero for the final triangle.
 * @status partial
 * @match 94.29
 * @residual 99/105 instructions at the original 420-byte size and 0x40 frame.
 * The next and wrapped angles occupy s3/s2 instead of the original s2/s3;
 * the current angle, loop setup and instruction ordering match.
 */
void func_801F2F04(s16 arg0, s16 arg1, s16 arg2, u8 arg3, u8 arg4) {
  s32 temp_lo_1;
  s32 temp_lo_2;
  s32 wrapped;
  s32 next;
  s32 angle;
  u8* primitive;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x3c0, 0), 0);
  func_8014E5A0(1, 0x0c);
  angle = 0;
  do {
    primitive = g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);
    do {
      next = angle + 0x80;
    } while (0);
    wrapped = next & 0xfff;
    {
      u16 current = angle;

      *(s16*)(primitive + 8) = arg0;
      *(s16*)(primitive + 0x0a) = arg1;
      *(s16*)(primitive + 0x10) = arg0 + ((arg2 * func_801783C8(current)) >> 12);
      *(s16*)(primitive + 0x12) = arg1 + ((arg2 * func_801782FC(current)) >> 12);
    }
    temp_lo_1 = arg2 * func_801783C8((u16)wrapped);
    angle = next;
    *(s16*)(primitive + 0x18) = arg0 + (temp_lo_1 >> 12);
    temp_lo_2 = arg2 * func_801782FC((u16)wrapped);
    primitive[4] = arg3;
    primitive[5] = arg3;
    primitive[6] = arg3;
    primitive[0x0c] = arg4;
    primitive[0x0d] = arg4;
    primitive[0x0e] = arg4;
    primitive[0x14] = arg4;
    primitive[0x15] = arg4;
    primitive[0x16] = arg4;
    *(s16*)(primitive + 0x1a) = arg1 + (temp_lo_2 >> 12);
    func_8014E5A0(1, 0x1c);
  } while ((u16)angle < 0x1000u);
}
