#include "internal.h"

/* does: walks the local eight-entry work array, runs the state handler selected
 * by byte `+0x01` for each active entry, then draws the entry and returns
 * whether any active work was processed.
 * @source: 0x801f30ec FUN_801f30ec
 */
s32 func_801f30ec(void) {
  u8  scratch[0x20];
  u8  i;
  s32 result;

  func_801afe18(scratch);

  result = 0;
  i = 0u;
  BOF3_WORLD00_AREA024_WORK_PTR = BOF3_WORLD00_AREA024_WORK_BASE;

  do {
    if (BOF3_WORLD00_AREA024_WORK_PTR[0] != 0u) {
      BOF3_WORLD00_AREA024_STATE_TABLE[BOF3_WORLD00_AREA024_WORK_PTR[1]]();
      func_801f2df8(BOF3_WORLD00_AREA024_WORK_PTR);
      result = 1;
    }

    BOF3_WORLD00_AREA024_WORK_PTR += 0x28u;
    i += 1u;
  } while (i < 8u);

  return result;
}
