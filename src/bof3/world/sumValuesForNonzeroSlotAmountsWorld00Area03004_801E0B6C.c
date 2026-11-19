#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E0B6C
 * @behavior Sums the values reported by func_801E0ABC for the nonzero amounts
 * among the 32 shared slot amount bytes at 0x80144FE4, and returns the
 * total masked to 16 bits.
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 sumValuesForNonzeroSlotAmountsWorld00Area03004(void) {
  s32 slotIndex;
  s32 totalValue = 0;

  for (slotIndex = 0; slotIndex < 0x20; slotIndex++) {
    u8 amount = D_80144FE4[slotIndex];

    if (amount != 0) {
      totalValue += func_801E0ABC(slotIndex & 0xff, amount);
    }
  }
  return totalValue & 0xffff;
}
