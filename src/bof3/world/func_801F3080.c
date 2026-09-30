#include "bof3/world/area02414_internal.h"

/* @behavior seeds the local eight-entry work array at `0x800e4800` and calls the
 * per-entry initializer for each `0x28`-byte slot.
 * @source 0x801F3080
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3080(void) {
  u8 i;

  workCursor = WORLD00_AREA024_WORK_BASE;
  i = 0u;

  do {
    func_801F2FD4(workCursor);
    i += 1u;
    workCursor += 0x28u;
  } while (i < 8u);
}
