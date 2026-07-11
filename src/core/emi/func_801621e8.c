#include "internal.h"

extern vu32 DAT_8014648c;
extern u8   DAT_80146498[];

/* @behavior latches the last CdSync callback result bytes and marks the async sync
 * status.
 * @source 0x801621e8 FUN_801621e8
 */
void func_801621e8(u8 status, u8* result) {
  bool has_more;
  s32  i;
  u8   value;
  u8*  src;

  i = 7;
  src = result + 7;

  do {
    value = *src;
    src -= 1;
    DAT_80146498[i] = value;
    has_more = i != 0;
    i -= 1;
  } while (has_more);

  if (status == CdlComplete) {
    DAT_8014648c = 1;
  } else {
    DAT_8014648c = -1;
  }
}
