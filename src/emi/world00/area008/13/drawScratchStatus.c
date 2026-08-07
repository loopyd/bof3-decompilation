#include "internal.h"

/* @behavior draws one local frame, formats the scratchpad pair into the shared UI
 * text buffer, then queues the three local UI labels.
 * @source 0x801F3C2C
 */
void drawScratchStatus(void) {
  World00Area008Scratch* scratch;
  char*                  buf;
  s32                    field_5d;
  s32                    field_5e;

  func_801F3D88(0x78, 0x38, 0x46, 0x14, 1u);

  scratch = (World00Area008Scratch*)WORLD00_AREA008_SCRATCH_PTR;
  field_5d = scratch->field_5d;
  field_5e = scratch->field_5e;
  buf = (char*)D_80145AD4;
  sprintf(buf, (const char*)D_801F2C04, field_5e,
          (field_5d * 100) / 30);

  func_8014FF0C(0x86, 0x3b, 0, buf);

  buf[0] = 0x3eu;
  buf[1] = 0u;

  func_8014F800(0x97, 0x35, 0, 0xffu, (u32)buf);
  func_8014F800(0x97, 0x39, 0, 0xffu, (u32)buf);
}
