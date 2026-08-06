#include "internal.h"

/* @behavior returns true only when the EXE-side EMI loader reached ready state 3.
 * @source 0x80162D00
 */
s32 emi_loader_is_ready(void) {
  volatile const u8* emi_state_base;

  emi_state_base = (volatile const u8*)0x80140000u;
  return emi_state_base[0x6494] == 3u;
}
