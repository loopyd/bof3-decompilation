#include "bof3/bof3.h"

/* Shared battle work pointer the main program publishes and its battle
 * overlays read back; the bmagic lifts read it as the shared battle work
 * record pointer and the battle03 lifts read the same cell as the local work
 * record pointer. Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, which the still-asm
 * neighbours func_800C1B14 and func_800C1BF0 keep alive, so no target-map row
 * is needed. Only the pointer value moves here, so the byte view is enough. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared readiness helper of the main executable. Its exact lifts
 * advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C and
 * waitReadyAdvanceSubstate3 read the returned byte as a ready flag, and the
 * boss-family lifts that drop it (accumulateEnemyThenRearmScratchState3
 * BossBoss02213_800C1938) call it without using the result. The original masks
 * the result with 0xFF before testing it, so the declared return type is the
 * byte it reports. */
u8 func_8014DAEC(void); /* @source 0x8014DAEC */

/* Shared main-executable resource request helper; the sibling boss and bmagic
 * lifts declare the same prototype and pass a resource id. */
void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* Scratchpad work-record pointer cell: the battle code publishes the scratch
 * work record here and every access in this payload reaches it through this
 * cell. The original re-reads the cell after the store and after each byte
 * store, because a store through the loaded pointer may alias the cell, and it
 * materializes the cell address per access (lui $vN,0x1F80 / lw/sw 68($vN));
 * the named symbol is the spelling that reproduces that, as in the exact
 * sibling applyMainLookupRecordF9ClearWorkByte24Bit0RequestResource2Then
 * AdvanceWorkByte2BossBoss01316_800C1D9C. The address-constant macro form
 * SPAD_PTR_SLOT is measured here to hoist the cell address into a saved
 * register instead, which diverges. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x800C1CE4
 * @behavior Step 2 handler of this overlay's step table at 0x800C1804 (entries
 *           0x800C1B14, 0x800C1BF0, 0x800C1CE4, 0x800C1D7C), which
 *           dispatchStateByte1Table1804BossBoss05116_800C1AA8 copies into its
 *           frame and indexes with the work record byte 1, so no jal in this
 *           payload reaches this entry. It saves the work pointer published at
 *           the scratchpad pointer cell 0x1F800044, publishes the shared battle
 *           work pointer D_801EB4E0 in that cell instead, calls the shared
 *           readiness helper at 0x8014DAEC and keeps the helper's byte result
 *           only as a ready test. While the helper reports not ready the
 *           handler writes nothing else; once it reports ready the handler
 *           requests resource 0 through the shared helper 0x8014D6B8, clears
 *           bit 0x80 of byte 0x24 of the shared battle work record the cell now
 *           names, stores 4 into that record's byte 0x29, and advances byte 1 of
 *           the saved overlay work record by one, so the byte-1 dispatch leaves
 *           this step for step 3 (0x800C1D7C) instead of re-entering it. Both
 *           paths then restore the saved pointer to the cell and return. The
 *           publish, poll and restore idiom matches the still-unlifted
 *           neighbouring step 0x800C1BF0, which publishes the same shared
 *           pointer into the same cell, runs the same readiness helper and
 *           restores the saved pointer before its own body; roles of record
 *           bytes 0x24 and 0x29 are not established here, and neither is the
 *           reason the helper is polled against the shared battle work record
 *           rather than the overlay's own. The record pointer load precedes
 *           frame creation, the 0x18-byte frame keeps $s0 at 0x10 and $ra at
 *           0x14 across the two calls, and the cell is re-read after each of
 *           the two byte stores because a store through the loaded pointer may
 *           alias the cell, so every access names the cell again.
 * @status exact
 * @match 100.00
 * @residual none
 */
void pollBattleWorkReadyThenRequestZeroClearByte24Bit7SetByte29To4AndAdvanceStepBossBoss05116_800C1CE4(
    void) {
  u8 *previous;

  previous = D_1F800044;
  D_1F800044 = D_801EB4E0;
  if (func_8014DAEC() != 0u) {
    func_8014D6B8(0);
    D_1F800044[0x24] &= 0x7Fu;
    D_1F800044[0x29] = 4;
    previous[1]++;
  }
  D_1F800044 = previous;
}
