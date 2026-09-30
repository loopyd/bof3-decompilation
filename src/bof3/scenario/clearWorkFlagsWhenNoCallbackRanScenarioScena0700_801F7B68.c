#include "bof3/bof3.h"

s32 func_801F85D8(void);
void func_80196070(void);

/* @source 0x801F7B68
 * @behavior Entry 2 of this overlay's three-entry work-state handler table at
 * 0x801FDDB4 (0x801F7A30 / 0x801F7AB0 / this function), which the dispatcher
 * dispatchWorkByte2HandlerScenarioScena0700_801F79EC indexes with the work-object byte at scratchpad pointer slot
 * 0x1F800044 + 0x02. It calls func_801F85D8, the sweeper over the eight
 * 0x1C-byte-stride entries of the 0x800E4800 work table that invokes each live
 * entry's callback from the pointer table at 0x8020DDF0 and returns 1 once at
 * least one callback ran, and, when the low byte of that result is zero (no
 * callback ran), calls the shared 0x80196070 work-area reset that clears
 * work-object bytes 0x00-0x04, including the dispatch byte 0x02. Takes no
 * arguments and returns nothing. The 52-byte body is byte-identical to the
 * already-exact sibling clearWorkFlagsWhenNoCallbackRanScenarioScena0700_801F6F14.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsWhenNoCallbackRanScenarioScena0700_801F7B68(void) {
  if ((func_801F85D8() & 0xFF) == 0) {
    func_80196070();
  }
}
