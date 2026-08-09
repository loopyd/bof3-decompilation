#include "bof3/world/area02414_internal.h"

/* @behavior called from the overlay frame entry func_801F2D5C: walks the local
 * eight-entry work array, runs the state handler selected
 * by byte `+0x01` for each active entry, then draws the entry and returns
 * whether any active work was processed.
 * @source 0x801F30EC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 dispatchWorkStates(void) {
  u8  scratch[0x20];
  u8  i;
  s32 result;

  func_801AFE18(scratch);

  result = 0;
  workCursor = WORLD00_AREA024_WORK_BASE;
  i = 0u;

  do {
    if (workCursor[0] != 0u) {
      stateTable[workCursor[1]]();
      func_801F2DF8(workCursor);
      result = 1;
    }

    workCursor += 0x28u;
    i += 1u;
  } while (i < 8u);

  return result;
}
