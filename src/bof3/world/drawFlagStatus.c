#include "bof3/world/area00813_internal.h"

/* @behavior draws one local frame, formats the single-byte world value into the
 * shared UI text buffer, then queues the matching local label.
 * @source 0x801F3D18
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void drawFlagStatus(void) {
  char* text_buffer;
  u8    value;

  drawTexturedFrame(200, 0xa0, 0x22, 0x14, 1u);
  value = D_80146867;
  text_buffer = (char*)D_80145AD4;
  sprintf(text_buffer, (const char*)D_801F2C10, value);
  func_8014FF0C(0xd1, 0xa3, 0, text_buffer);
}
