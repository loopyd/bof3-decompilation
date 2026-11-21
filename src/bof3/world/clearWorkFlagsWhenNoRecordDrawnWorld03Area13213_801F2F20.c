#include "bof3/bof3.h"

u32 tickAndDrawActiveWorkRecordsWorld03Area13213_801F38B0(void);
void func_80196070(void);

/* @source 0x801F2F20
 * @behavior Entry 2 of the byte-2 handler pointer run based at 0x801F3DEC - the
 *           word at 0x801F3DF4 - which the dispatcher at 0x801F2DF4 indexes
 *           with the work record's byte at +0x02. It runs this overlay's own
 *           record update and draw pass at 0x801F38B0, masks that pass's
 *           returned byte with 0xFF and, on the frames where the masked result
 *           is zero, calls the shared work-area reset at 0x80196070. The pass
 *           reports one once it advanced or drew a claimed record of the
 *           0x20-entry, 0x18-byte-stride table at 0x800E4800 and zero when the
 *           whole table stayed unclaimed, so the zero arm is the idle table,
 *           and 0x80196070 clears work-object bytes 0x00-0x04 and so returns
 *           the overlay to work state 0. This entry takes no call argument,
 *           returns nothing, and its 0x18-byte frame holds only $ra across the
 *           two calls.
 *
 * Name evidence: the only in-image word naming 0x801F2F20 is the pointer word
 * at 0x801F3DF4, index 2 of the run based at 0x801F3DEC; no jal in the payload
 * reaches this address, so that run is the whole of its call evidence. The
 * hand-off into this entry is recorded by the already exact sibling entry 1 at
 * 0x801F2E88, which increments the work byte at +0x02 so the next step
 * dispatches this address, and which described it as re-running the record pass
 * and calling func_80196070 when that pass reports nothing left to draw. The
 * reset role of 0x80196070 is the same one its exact callers in this repository
 * record - the work-area reset that clears work-object bytes 0x00-0x04 and
 * returns the overlay to work state 0 - so the name states the guard (no record
 * drawn) and the action (clearing the work flags) the body performs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsWhenNoRecordDrawnWorld03Area13213_801F2F20(void) {
  if ((tickAndDrawActiveWorkRecordsWorld03Area13213_801F38B0() & 0xFF) == 0) {
    func_80196070();
  }
}
