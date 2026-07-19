#include "internal.h"

extern void func_8014D6B8(u32 flag);

/* @behavior Selects disabled mode when the shared high-bit flag is set;
 * otherwise installs and enables the area resource, then selects mode 2.
 * @source 0x801F2C88
 */
void func_801F2C88(void) {
  volatile World00Area008State* previous;

  if ((WORLD00_AREA008_D_80146867 & 0x80u) != 0u) {
    WORLD00_AREA008_SCRATCH_PTR->mode = 9;
    return;
  }

  previous = WORLD00_AREA008_SCRATCH_PTR;
  WORLD00_AREA008_STATE_PTR = WORLD00_AREA008_STATE_BASE;
  WORLD00_AREA008_SCRATCH_PTR = WORLD00_AREA008_STATE_BASE;
  REG8(0x801460e8) |= 0x40;
  func_8014D6B8(0x10);
  REG8(0x80146866) = 1;
  WORLD00_AREA008_SCRATCH_PTR = previous;
  previous->mode = 2;
}
