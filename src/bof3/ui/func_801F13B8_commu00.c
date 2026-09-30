#include "bof3/ui/commu00_internal.h"

extern int sprintf(char* buffer, const char* format, ...);

/* @source 0x801F13B8
 * @behavior Formats the shared counter words at 0x80145034, 0x80145038 and
 *           0x8014503C through the container-local decimal format string into
 *           the three 0x20-strided slots of the D_801490D8 text buffer, starts
 *           action 0x56 on the frontend task, and latches the shared phase byte
 *           to 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801F13B8(void) {
  char* buffer;
  const char* format;

  buffer = (char*)D_801490D8;
  format = (const char*)D_801EEC44;
  sprintf(buffer, format, D_80145034);
  sprintf(buffer + 0x20, format, D_80145038);
  sprintf(buffer + 0x40, format, D_8014503C);
  func_80150224(0x56);
  D_80143BB0 = 2;
  return 0;
}
