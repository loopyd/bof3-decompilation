#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E1C10
 * @behavior Scans 128 slots for each candidate value 0x38 through 0x4D, counting
 * slots whose byte at 0x8014504C matches that value and whose
 * companion byte at 0x8014524C is 0x63. Reports whether at least 22
 * slots match; duplicate values in separate slots count separately.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 hasAtLeastTwentyTwoMatchingSlotsWorld00Area03004(void) {
  s32 candidateValue = 0x38;
  s32 matchingCount = 0;
  s32 slotIndex;

  for (; candidateValue < 0x4E; candidateValue++) {
    for (slotIndex = 0; slotIndex < 0x80; slotIndex++) {
      if (D_8014504C[slotIndex] == candidateValue && D_8014524C[slotIndex] == 0x63) {
        matchingCount = matchingCount + 1;
      }
    }
  }
  return matchingCount >= 0x16;
}
