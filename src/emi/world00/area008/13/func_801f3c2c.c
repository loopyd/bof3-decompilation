#include "internal.h"

/* @behavior draws one local frame, formats the scratchpad pair into the shared UI
 * text buffer, then queues the three local UI labels.
 * @source 0x801f3c2c FUN_801f3c2c
 */
void func_801f3c2c(void) {
  World00Area008Scratch* scratch;
  char*                  buf;
  s32                    field_5d;
  s32                    field_5e;

  func_801f3d88(0x78, 0x38, 0x46, 0x14, 1u);

  scratch = (World00Area008Scratch*)WORLD00_AREA008_SCRATCH_PTR;
  field_5d = scratch->field_5d;
  field_5e = scratch->field_5e;
  buf = (char*)WORLD00_AREA008_DAT_80145AD4;
  func_8017e3f4(buf, (const char*)WORLD00_AREA008_DAT_801F2C04, field_5e,
                (field_5d * 100) / 30);

  func_8014ff0c(0x86, 0x3b, 0, buf);

  buf[0] = 0x3eu;
  buf[1] = 0u;

  func_8014f800(0x97, 0x35, 0, 0xffu, (u32)buf);
  func_8014f800(0x97, 0x39, 0, 0xffu, (u32)buf);
}
