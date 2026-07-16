#include "internal.h"

/* @behavior seeds the local eight-entry work array at `0x800e4800` and calls the
 * per-entry initializer for each `0x28`-byte slot.
 * @source 0x801F3080
 */
void func_801F3080(void) {
  u8 i;

  i = 0u;
  WORLD00_AREA024_WORK_PTR = WORLD00_AREA024_WORK_BASE;

  do {
    func_801F2FD4(WORLD00_AREA024_WORK_PTR);
    WORLD00_AREA024_WORK_PTR += 0x28u;
    i += 1u;
  } while (i < 8u);
}
