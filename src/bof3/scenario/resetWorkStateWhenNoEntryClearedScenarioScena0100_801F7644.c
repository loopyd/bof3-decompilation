#include "bof3/bof3.h"

s32 func_801F7C54(void);
void func_80196070(void);

/* @source 0x801F7644
 * @behavior Last per-frame handler of this overlay's work-state machine: the
 * address is entry 4, the fifth word at 0x801FE27C, of the overlay handler table
 * at 0x801FE26C that dispatchWorkByte1HandlerScenarioScena0100_801F73B8 indexes
 * with byte 1 of the scratchpad work object at 0x1F800044. It runs the work-table
 * sweeper func_801F7C54, which returns non-zero in the frames where it retired at
 * least one 0x14-byte entry of the 0x800E4800 work table, and when the result
 * masked to a byte is zero it resets the work object's state through the shared
 * func_80196070 helper, which zeroes work-object bytes 0x00-0x04 - including the
 * dispatch byte 1 - and so returns the overlay to work state 0. It takes no
 * arguments, returns nothing, reads and writes no other state, and the 0x18-byte
 * frame only keeps $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetWorkStateWhenNoEntryClearedScenarioScena0100_801F7644(void) {
  if ((func_801F7C54() & 0xFF) == 0) {
    func_80196070();
  }
}
