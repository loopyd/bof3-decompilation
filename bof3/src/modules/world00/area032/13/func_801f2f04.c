#include "internal.h"

/* does: allocates one translucent draw-mode primitive, then emits a 32-step
 * gouraud triangle ring around `(arg0, arg1)` using `arg2` as the radius.
 * @source: 0x801f2f04 FUN_801f2f04
 */
void func_801f2f04(s16 arg0, s16 arg1, s16 arg2, u8 arg3, u8 arg4) {
  u32 primitive;
  s16 radius;
  u16 angle;
  u16 next_angle;
  u16 wrapped_angle;
  s32 trig_value;
  u16 tpage;

  tpage = GetTPage(0, 1, 0x3c0, 0);
  SetDrawMode((DR_MODE*)BOF3_WORLD00_AREA032_13_PRIMITIVE_PTR, 0, 1, tpage,
              NULL);
  func_8014e5a0(1u, 0x0cu);

  radius = arg2;
  angle = 0u;
  do {
    primitive = (u32)BOF3_WORLD00_AREA032_13_PRIMITIVE_PTR;
    SetPolyG3((POLY_G3*)primitive);
    SetSemiTrans((void*)primitive, 1);

    next_angle = angle + 0x80u;
    wrapped_angle = next_angle & 0xfffu;
    angle &= 0xffffu;

    *(volatile s16*)(primitive + 8) = arg0;
    trig_value = rcos(angle);
    *(volatile s16*)(primitive + 0x0au) = arg1;
    *(volatile s16*)(primitive + 0x10u) =
        arg0 + (s16)(((s32)radius * trig_value) >> 12);

    trig_value = rsin(angle);
    *(volatile s16*)(primitive + 0x12u) =
        arg1 + (s16)(((s32)radius * trig_value) >> 12);

    wrapped_angle &= 0xffffu;
    trig_value = rcos(wrapped_angle);
    *(volatile s16*)(primitive + 0x18u) =
        arg0 + (s16)(((s32)radius * trig_value) >> 12);

    angle = next_angle;
    trig_value = rsin(wrapped_angle);
    *(volatile u8*)(primitive + 4) = arg3;
    *(volatile u8*)(primitive + 5) = arg3;
    *(volatile u8*)(primitive + 6) = arg3;
    *(volatile u8*)(primitive + 0x0cu) = arg4;
    *(volatile u8*)(primitive + 0x0du) = arg4;
    *(volatile u8*)(primitive + 0x0eu) = arg4;
    *(volatile u8*)(primitive + 0x14u) = arg4;
    *(volatile u8*)(primitive + 0x15u) = arg4;
    *(volatile u8*)(primitive + 0x16u) = arg4;
    *(volatile s16*)(primitive + 0x1au) =
        arg1 + (s16)(((s32)radius * trig_value) >> 12);
    func_8014e5a0(1u, 0x1cu);
  } while (next_angle < 0x1000u);
}
