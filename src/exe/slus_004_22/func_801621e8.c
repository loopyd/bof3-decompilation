#include "internal.h"

extern vu32 D_8014648C;
extern u8   D_80146498[];

/* @behavior latches the last CdSync callback result bytes and marks the async sync
 * status.
 * @source 0x801621e8 FUN_801621e8
 */
void func_801621e8(s32 status, u8* result) {
  s32 i;

  result += (i = 7);

  do {
    /* The folded pair preserves the original GCC setup scheduling. */
    i++;
    i--;
    D_80146498[i] = *result--;
  } while (i-- != 0);

  if (status == CdlComplete) {
    D_8014648C = 1;
  } else {
    D_8014648C = -1;
  }
}
