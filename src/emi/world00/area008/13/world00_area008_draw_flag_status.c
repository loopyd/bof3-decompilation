#include "internal.h"

/* @behavior draws one local frame, formats the single-byte world value into the
 * shared UI text buffer, then queues the matching local label.
 * @source 0x801F3D18
 */
void world00_area008_draw_flag_status(void) {
  char* text_buffer;
  u8    value;

  func_801F3D88(200, 0xa0, 0x22, 0x14, 1u);
  value = WORLD00_AREA008_D_80146867;
  text_buffer = (char*)WORLD00_AREA008_D_80145AD4;
  sprintf(text_buffer, (const char*)WORLD00_AREA008_D_801F2C10, value);
  func_8014FF0C(0xd1, 0xa3, 0, text_buffer);
}
