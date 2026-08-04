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
  D_801F5B00 = WORLD00_AREA024_WORK_BASE;
  i = 0u;

  do {
    if (D_801F5B00[0] != 0u) {
      D_801F4214[D_801F5B00[1]]();
      func_801F2DF8(D_801F5B00);
      result = 1;
    }

    D_801F5B00 += 0x28u;
    i += 1u;
  } while (i < 8u);

  return result;
}
