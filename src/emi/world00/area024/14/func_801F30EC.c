#include "internal.h"

/* @behavior walks the local eight-entry work array, runs the state handler selected
 * by byte `+0x01` for each active entry, then draws the entry and returns
 * whether any active work was processed.
 * @source 0x801F30EC
 */
s32 func_801F30EC(void) {
  u8  scratch[0x20];
  u8  i;
  s32 result;

  func_801AFE18(scratch);

  result = 0;
  i = 0u;
  WORLD00_AREA024_WORK_PTR = WORLD00_AREA024_WORK_BASE;

  do {
    if (WORLD00_AREA024_WORK_PTR[0] != 0u) {
      WORLD00_AREA024_STATE_TABLE[WORLD00_AREA024_WORK_PTR[1]]();
      func_801F2DF8(WORLD00_AREA024_WORK_PTR);
      result = 1;
    }

    WORLD00_AREA024_WORK_PTR += 0x28u;
    i += 1u;
  } while (i < 8u);

  return result;
}
