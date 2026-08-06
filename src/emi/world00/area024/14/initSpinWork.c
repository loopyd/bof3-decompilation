#include "internal.h"

/* @behavior called from the overlay init entry func_801F2C58: seeds the local
 * 16-entry spin-work table at `0x800e5000` by calling
 * the per-entry initializer on each `0x2c`-byte slot.
 * @source 0x801F3D0C
 */
void initSpinWork(void) {
  u8* work;
  u8  i;

  work = WORLD00_AREA024_SPIN_WORK_BASE;
  i = 0u;

  do {
    func_801F3BE4(work);
    work += 0x2cu;
    i += 1u;
  } while (i < 0x10u);
}
