#include "internal.h"

/* @behavior seeds the local 16-entry spin-work table at `0x800e5000` by calling
 * the per-entry initializer on each `0x2c`-byte slot.
 * @source 0x801f3d0c FUN_801f3d0c
 */
void func_801f3d0c(void) {
  u8* work;
  u8  i;

  work = WORLD00_AREA024_SPIN_WORK_BASE;
  i = 0u;

  do {
    func_801f3be4(work);
    work += 0x2cu;
    i += 1u;
  } while (i < 0x10u);
}
