#include "internal.h"

/* @behavior called from the overlay frame entry func_801F2D5C: walks the local
 * eight-entry work array, runs the state handler selected
 * by byte `+0x01` for each active entry, then draws the entry and returns
 * whether any active work was processed.
 * @source 0x801F30EC
 */
s32 world00_area024_dispatch_work_states(void) {
  u8  scratch[0x20];
  u8  i;
  s32 result;

  func_801AFE18(scratch);

  result = 0;
  world00_area024_work_cursor = WORLD00_AREA024_WORK_BASE;
  i = 0u;

  do {
    if (world00_area024_work_cursor[0] != 0u) {
      world00_area024_state_table[world00_area024_work_cursor[1]]();
      func_801F2DF8(world00_area024_work_cursor);
      result = 1;
    }

    world00_area024_work_cursor += 0x28u;
    i += 1u;
  } while (i < 8u);

  return result;
}
