#include "bof3/bof3.h"

s32 func_801F9EFC(void);
void func_80196070(void);

/* @source 0x801F75C0
 * @behavior Entry 2 of this overlay's three-entry work-state handler table at
 * 0x801FDD70 (0x801F7408 / 0x801F7480 / this function), which the dispatcher
 * func_801F73C4 indexes with the work-object byte at scratchpad pointer slot
 * 0x1F800044 + 0x02. It runs func_801F9EFC, the per-frame pass over the
 * sixty-four 0x20-byte-stride entries of the 0x800E515C work table - that pass
 * draws and advances every entry whose lead byte is set and clears the lead
 * byte when the entry's countdown expires - and, when the low byte of its
 * result is zero (this frame left no live entry in that work table), calls the
 * shared 0x80196070 work-area reset that clears work-object bytes 0x00-0x04,
 * including the dispatch byte 0x02. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkStateWhenWorkTableIdleScenarioScena0700_801F75C0(void) {
  if ((func_801F9EFC() & 0xFF) == 0) {
    func_80196070();
  }
}
