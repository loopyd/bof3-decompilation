#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E1C10
 * @behavior Scans the 0x80-entry AREA030 slot tables for slots whose value
 * byte 0x8014504C equals the candidate 0x38..0x4D and whose companion byte
 * 0x8014524C holds the sentinel 0x63, and reports whether at least 22 such
 * slots exist.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 27/27 instructions, 108 bytes, live byte match.
 */
s32 func_801E1C10(void) {
  s32 i = 0x38;
  s32 count = 0;
  s32 j;

  for (; i < 0x4E; i++) {
    for (j = 0; j < 0x80; j++) {
      if (D_8014504C[j] == i && D_8014524C[j] == 0x63) {
        count = count + 1;
      }
    }
  }
  return count >= 0x16;
}
