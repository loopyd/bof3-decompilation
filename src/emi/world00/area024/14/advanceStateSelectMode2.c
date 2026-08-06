#include "internal.h"

/* @behavior mode-2 handler of the work state dispatch table
 * (stateTable): increments the byte at offset 1 of the
 * current work pointer.
 * @source 0x801F32D4
 */
void advanceStateSelectMode2(void) {
  u8* work;

  work = WORLD00_AREA024_WORK_PTR;
  work[1]++;
}
