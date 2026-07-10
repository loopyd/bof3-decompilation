#include "internal.h"

/* does: counts the active COMMU00 source slots in the 0x3c-entry source table.
 * @source: 0x801f02e4 FUN_801f02e4
 */
u8 func_801f02e4(void) {
  u8  count;
  s32 offset;

  count = 0u;
  offset = 0;
  do {
    if (((const volatile u8*)0x801455c8u)[offset] != 0u) {
      count += 1u;
    }
    offset += 8;
  } while (offset < 0x1e0);

  return count;
}
