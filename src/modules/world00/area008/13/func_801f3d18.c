#include "internal.h"

/* @behavior draws one local frame, formats the single-byte world value into the
 * shared UI text buffer, then queues the matching local label.
 * @source 0x801f3d18 FUN_801f3d18
 */
void func_801f3d18(void) {
  func_801f3d88(200, 0xa0, 0x22, 0x14, 1u);
  func_8017e3f4((void*)WORLD00_AREA008_UI_CHAR_BUFFER, (const void*)0x801f2c10u,
                WORLD00_AREA008_GLOBAL_BYTE_6867);
  func_8014ff0c(0xd1, 0xa3, 0, (const void*)WORLD00_AREA008_UI_CHAR_BUFFER);
}
