#include "internal.h"

/* does: draws one local frame, formats the scratchpad pair into the shared UI
 * text buffer, then queues the three local UI labels.
 * @source: 0x801f3c2c FUN_801f3c2c
 */
void func_801f3c2c(void) {
  volatile World00Area008Scratch* scratch;

  func_801f3d88(0x78, 0x38, 0x46, 0x14, 1u);

  scratch = WORLD00_AREA008_SCRATCH_PTR;
  func_8017e3f4((void*)WORLD00_AREA008_UI_CHAR_BUFFER, (const void*)0x801f2c04u,
                (s32)scratch->field_5e, ((s32)scratch->field_5d * 100) / 30);

  func_8014ff0c(0x86, 0x3b, 0, (const void*)WORLD00_AREA008_UI_CHAR_BUFFER);

  WORLD00_AREA008_UI_CHAR_BUFFER[0] = 0x3eu;
  WORLD00_AREA008_UI_CHAR_BUFFER[1] = 0u;

  func_8014f800(0x97, 0x35, 0, 0xffu, (u32)WORLD00_AREA008_UI_CHAR_BUFFER);
  func_8014f800(0x97, 0x39, 0, 0xffu, (u32)WORLD00_AREA008_UI_CHAR_BUFFER);
}
