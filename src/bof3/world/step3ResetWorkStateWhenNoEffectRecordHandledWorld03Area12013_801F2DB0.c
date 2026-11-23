#include "bof3/bof3.h"

/* Target-local effect sweep at 0x801F36B0, lifted exact from this target's
 * assembly before this handler was seeded, so only its prototype is declared
 * here. It walks the 256 entries of the effect table at 0x800E4800 and returns
 * 1 when at least one entry was handled and 0 otherwise. */
s32 advanceWorkRecordStatesAndRenderActiveWorld03Area12013_801F36B0(void);

/* Shared work-area reset helper at 0x80196070, outside this overlay's payload
 * in the main executable. It clears the work-state header bytes 0x00-0x04 of
 * the scratchpad work object, including byte 0x01. */
void func_80196070(void);

/* @source 0x801F2DB0
 * @behavior Step 3 of the run this overlay dispatches through the handler
 *           table at 0x801F43C0: the exact dispatcher at 0x801F2C18 indexes
 *           that table with work record byte 0x01, and entry 3 of the table
 *           holds this address. The body first runs the target-local effect
 *           sweep advanceWorkRecordStatesAndRenderActiveWorld03Area12013_801F36B0
 *           over the 256 entries of the effect table at 0x800E4800; only when
 *           that result masked to a byte reads zero, meaning no entry was
 *           handled this frame, does it call the shared work-area reset helper
 *           func_80196070, which clears work record bytes 0x00-0x04 and so
 *           returns the overlay to step 0 of the same table. Takes no
 *           arguments, returns nothing, and its 0x18-byte frame holds only
 *           the saved return address at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void step3ResetWorkStateWhenNoEffectRecordHandledWorld03Area12013_801F2DB0(void) {
  if ((advanceWorkRecordStatesAndRenderActiveWorld03Area12013_801F36B0() & 0xFF) ==
      0) {
    func_80196070();
  }
}
