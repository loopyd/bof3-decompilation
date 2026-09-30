#include "bof3/ui/game00_internal.h"

extern void func_801C57B4(void);

/*
 * @source 0x801B0DFC
 * @behavior Advances the work record position by its current delta step:
 * func_801C57B4 adds the work-area words at 0x0C and 0x10 and the packed word
 * at 0x14 into the coordinate words at 0x34 / 0x38 and the counter halfword at
 * 0x3E. The shared no-argument update func_8014D978 is then run only when the
 * shared-work mode byte at D_80146250 + 0x11C is at least 4 (unsigned compare),
 * so the mode values 0 to 3 skip that update.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkThenConditionalSharedUpdate_game00_801B0DFC(void) {
  func_801C57B4();
  if (D_80146250[0x11C] >= 4u) {
    func_8014D978();
  }
}
