#include "bof3/bof3.h"

s32 func_80117410(void);
void func_80196070(void);

/* @source 0x801170A4
 * @behavior State-1 handler of this overlay's two-word per-frame state-handler
 * table at 0x8011818C (its word 0 is func_80117048, the boot handler that
 * installs the 0x548000 / 0x0B8000 / 0x01000000 clip values, calls func_801172D0
 * and finally increments byte 1 of the scratchpad work object at 0x1F800044 so
 * that the dispatcher func_80117004 selects this word). It runs the work-record
 * pass func_80117410, which walks the 16 records of 0x28-byte stride in the
 * 0x800E4800 work table, runs each claimed record's per-frame handler from the
 * overlay table at 0x80118194 (indexed by the record's byte 1) followed by the
 * record renderer func_801170D8, and leaves 1 in its return register once it has
 * handled at least one record and 0 when no record was claimed; when that result
 * masked to its low byte is zero the shared work-area reset func_80196070
 * (clearWorkFlags in emi/etc/game/00) clears work-object bytes 0x00-0x04 and so
 * returns the overlay to work state 0. Takes no arguments and returns nothing;
 * the 0x18-byte frame only keeps $ra across the two calls and the second call is
 * reached only on the zero path. Its 0x34 bytes are the same instruction shape
 * as the exact sibling resetWorkStateWhenNoEntryClearedScenarioScena0100_801F7644.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsWhenNoEntryHandledWorld01Area04913_801170A4(void) {
  if ((func_80117410() & 0xFF) == 0) {
    func_80196070();
  }
}
