#include "bof3/world/area00813_internal.h"

/* @source 0x801F437C
 * @behavior formats the masked shared world byte through the local `%d` format
 *           string into the shared UI text buffer, starts action 0x54 on the
 *           selected master, then records phase 2 in the shared phase byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F437C(void) {
  char* text_buffer;
  u8    value;

  text_buffer = (char*)D_801490D8;
  value = D_80146867 & 0x7F;
  sprintf(text_buffer, (const char*)D_801F2C14, value);
  func_80150224(0x54);
  D_80143BB0 = 2;
}
