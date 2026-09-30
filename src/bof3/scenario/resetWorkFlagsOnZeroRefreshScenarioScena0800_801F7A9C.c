#include "bof3/bof3.h"

u32 func_801F8BF8(void);
void func_80196070(void);

/* @source 0x801F7A9C
 * @behavior Overlay-local conditional work-flag reset: calls the table-refresh
 * helper at 0x801F8BF8, which walks the 0x8 entries of the 0x1C-byte-stride
 * record table at 0x800E4800 and leaves a flag word in its return register
 * (cleared at entry, set to 1 when an entry takes its per-entry handler call),
 * masks that result to its low byte and calls the shared work-area reset
 * helper at 0x80196070 (the address mapped as clearWorkFlags in
 * emi/etc/game/00) only when that low byte is zero. It takes no arguments,
 * returns nothing, reads no state of its own and writes none beyond what the
 * two callees touch; the 0x18-byte frame only keeps $ra across the calls and
 * the second call is reached only on the zero path. The callee prototype below
 * is the caller-side width: only the low byte of the result is consumed. Its
 * 0x34 bytes are the same shape as the sibling at 0x801F7048 in this payload,
 * which tests the same low byte with the opposite branch.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkFlagsOnZeroRefreshScenarioScena0800_801F7A9C(void) {
  if ((u8)func_801F8BF8() == 0) {
    func_80196070();
  }
}
