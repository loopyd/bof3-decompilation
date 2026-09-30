#include "bof3/bof3.h"

s32 func_801F85D8(void);
void func_80196070(void);

/* @source 0x801F6F14
 * @behavior Third pointer of the overlay's three-entry handler table at
 * 0x801FDD3C. It calls the helper at 0x801F85D8 - which walks the eight
 * 0x1C-byte-stride entries of the 0x800E4800 work table and returns 1 once it
 * has invoked at least one live entry's callback from the table at 0x8020DDF0
 * - and, when the low byte of that result is zero (no callback ran), calls the
 * shared scratchpad work-area reset helper at 0x80196070. It takes no
 * arguments, returns nothing, and touches no game state itself.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsWhenNoCallbackRanScenarioScena0700_801F6F14(void) {
  if ((func_801F85D8() & 0xFF) == 0) {
    func_80196070();
  }
}
