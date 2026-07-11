#include "internal.h"

/* @behavior draws one local frame, formats the single-byte world value into the
 * shared UI text buffer, then queues the matching local label.
 * @source 0x801f3d18 FUN_801f3d18
 */
void func_801f3d18(void) {
  char* text_buffer;

  func_801f3d88(200, 0xa0, 0x22, 0x14, 1u);
  text_buffer = (char*)WORLD00_AREA008_DAT_80145AD4;
  func_8017e3f4(text_buffer, (const char*)WORLD00_AREA008_DAT_801F2C10,
                WORLD00_AREA008_DAT_80146867);
  func_8014ff0c(0xd1, 0xa3, 0, text_buffer);
}
