#include "bof3/ui/commu00_internal.h"

extern int sprintf(char* buffer, const char* format, ...);

/* @source 0x801F1318
 * @behavior Formats the container-local byte at 0x80144FC0, the byte at
 *           0x80144FC1, the shared counter word at 0x8014502C and the shared
 *           counter word at 0x80145030 through the container-local decimal
 *           format string into the four 0x20-strided slots of the D_801490D8
 *           text buffer, starts action 0x55 on the frontend task, and latches
 *           the shared phase byte to 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801F1318(void) {
  char* buffer;
  const char* format;

  buffer = (char*)D_801490D8;
  format = (const char*)D_801EEC44;
  sprintf(buffer, format, D_80144FC0);
  sprintf(buffer + 0x20, format, D_80144FC1);
  sprintf(buffer + 0x40, format, D_8014502C);
  sprintf(buffer + 0x60, format, D_80145030);
  func_80150224(0x55);
  D_80143BB0 = 2;
  return 0;
}
