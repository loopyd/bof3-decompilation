#include "internal.h"

/* does: seeds the local eight-entry work array at `0x800e4800` and calls the
 * per-entry initializer for each `0x28`-byte slot.
 * @source: 0x801f3080 FUN_801f3080
 */
void func_801f3080(void) {
  u8 i;

  i = 0u;
  WORLD00_AREA024_WORK_PTR = WORLD00_AREA024_WORK_BASE;

  do {
    func_801f2fd4(WORLD00_AREA024_WORK_PTR);
    WORLD00_AREA024_WORK_PTR += 0x28u;
    i += 1u;
  } while (i < 8u);
}
