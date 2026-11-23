#include "bof3/bof3.h"

u32 tickDrawActiveRecords(void);
void func_80196070(void);

/* @source 0x801F3048
 * @behavior Entry 1 of the byte-2 handler pointer run based at 0x801F3DF8 - the
 *           word at 0x801F3DFC - which the dispatcher at 0x801F2F54 indexes with
 *           the work record's byte at +0x02. It runs this overlay's own record
 *           update and draw pass at 0x801F38B0, masks that pass's returned byte
 *           with 0xFF and, on the frames where the masked result is zero, calls
 *           the shared work-area reset at 0x80196070. The pass reports one once
 *           it advanced or drew a claimed record of the 0x20-entry,
 *           0x18-byte-stride table at 0x800E4800 and zero when the whole table
 *           stayed unclaimed, so the zero arm is the idle table, and 0x80196070
 *           clears work-object bytes 0x00-0x04 and so returns the overlay to
 *           work state 0. This entry takes no call argument, returns nothing,
 *           and its 0x18-byte frame holds only $ra across the two calls.
 *
 * Name evidence: the span is byte for byte equal to the already exact entry 2
 * of the byte-2 run at 0x801F3DEC, 0x801F2F20, whose lifted source at
 * src/bof3/world/stateFlagsNoDrawn.c
 * records this same guard and reset; one payload compare over its 0x34 bytes
 * proved the two spans identical, so the body facts here are that entry's facts.
 * The only word in the image that names 0x801F3048 is the pointer word at
 * 0x801F3DFC, index 1 of the run based at 0x801F3DF8, and no jal in the payload
 * targets this address, so that run is the whole of its reference evidence. The
 * Table3df8 qualifier keeps this duplicate distinct from the 0x801F2F20 entry
 * and names the run that consumes it, matching this target's own
 * dispatchStateByte2Table3df8WorldWorld03Area13213_801F2F54 spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsWhenNoRecordDrawnTable3df8World03Area13213_801F3048(void) {
  if ((tickDrawActiveRecords() & 0xFF) == 0) {
    func_80196070();
  }
}
