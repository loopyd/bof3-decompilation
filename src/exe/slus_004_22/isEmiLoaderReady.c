#include "internal.h"

/* @behavior returns true only when the EXE-side EMI loader reached ready state 3.
 * @source 0x80162D00
 */
s32 isEmiLoaderReady(void) {
  volatile const u8* emiStateBase;

  emiStateBase = (volatile const u8*)0x80140000u;
  return emiStateBase[0x6494] == 3u;
}
