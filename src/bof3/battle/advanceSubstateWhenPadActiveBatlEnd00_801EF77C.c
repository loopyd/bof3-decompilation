#include "bof3/bof3.h"

extern u16 D_80145AA4;
extern u8 D_801462E4;
extern u8 D_801462E5;

/* @source 0x801EF77C
 * @behavior Advances the battle substate index at 0x801462E4 by one and clears
 *           the battle state byte at 0x801462E5 (g_BtlState in
 *           include/battle/ram.h) whenever the shared pad state word at
 *           0x80145AA4 reads nonzero; returns nothing and touches no stack.
 * @status exact
 * @match 100.00
 * @residual none
 * The state byte cleared here is the value the substate table dispatchers use as
 * a per-entry latch, and 0x801462E4 is the index they scale into the handler run
 * (dispatchSubstateTable0cc0BatlEnd00_801EF30C).
 */
void advanceSubstateWhenPadActiveBatlEnd00_801EF77C(void) {
  if (D_80145AA4 != 0) {
    D_801462E5 = 0;
    D_801462E4 += 1;
  }
}
