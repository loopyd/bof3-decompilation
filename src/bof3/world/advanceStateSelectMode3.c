#include "bof3/world/area02414_internal.h"

/* @behavior mode-3 handler of the work state dispatch table
 * (stateTable): increments the byte at offset 1 of the
 * current work pointer.
 * @source 0x801F32F4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advanceStateSelectMode3(void) {
  u8* work;

  work = WORLD00_AREA024_WORK_PTR;
  work[1]++;
}
