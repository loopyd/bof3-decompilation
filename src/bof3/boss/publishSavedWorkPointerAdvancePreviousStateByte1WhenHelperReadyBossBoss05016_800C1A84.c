#include "bof3/bof3.h"

/* Active work-object pointer cell of the scratchpad at 0x1F800044, named
 * g_battle_work by the battle, bmagic, plchar and ui lifts of this project and
 * bound by a row of this target's own map. The original loads the cell once
 * before the swap and re-materialises the cell address with its own lui for
 * each of the two stores, so the cell is reached through this named pointer
 * rather than through a folded constant base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM at 0x801EC2E0, outside this overlay
 * image and bound by this target's generated undefined_syms_auto.txt. Word 0x74
 * of that record is the saved work-object pointer: the boss sibling
 * publishSavedWorkPointerSelectSequence2RestorePreviousThenSetWorkWordCTo8And
 * AdvanceStateByte1BossBoss05416_800C1A54 publishes exactly that word into the
 * scratchpad cell, and this payload's own step 0x800C1A08 does the same before
 * calling its helpers. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Shared main-program readiness helper of the main executable at 0x8014DAEC,
 * outside this overlay image and bound by this target's generated
 * undefined_funcs_auto.txt. The boss sibling setWorkByte1To2WhenHelperReady
 * BossBoss02213_800C18B8 and the bmagic lifts at 0x801EEE50, 0x801EEE74 and
 * 0x801EEEDC declare the same zero-extended byte prototype, and every one of
 * those sites tests only the returned byte, so its role stays address-anchored
 * and no map row is added here. */
extern u8 func_8014DAEC(void);

/* @source 0x800C1A84
 * @behavior Step handler of this overlay work state machine: it publishes word
 *           0x74 of the shared battle record 0x801EC2E0, that record's saved
 *           work-object pointer, into the scratchpad work-object pointer cell
 *           0x1F800044 while keeping the cell's previous value in a
 *           callee-saved register, calls the shared readiness helper
 *           func_8014DAEC and tests only the zero-extended low byte of its
 *           answer; on non-zero it increments byte 1 of the work object that
 *           was published before this call, on zero it writes nothing more, and
 *           either way it restores the cell to the saved pointer. Byte 1 of the
 *           record at that cell is the dispatch state byte this overlay steps on
 *           - dispatchStateByte1Table1b4cBossBoss05016_800C1810 loads it, scales
 *           it by four and calls that entry of the handler run at 0x800C1B4C -
 *           so the increment advances the dispatch state of the previous work
 *           object. The helper and the increment therefore both run while the
 *           record's own saved pointer is published into the cell. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps the
 *           saved cell value at 0x10 and the return address at 0x14 across the
 *           one call. The payload names this span only through word 1 of the
 *           three-word pointer run at 0x800C1804, whose entries are this
 *           overlay's steps 0x800C1A08, 0x800C1A84 and 0x800C1AEC, so the
 *           overlay loader that steps that run is its caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishSavedWorkPointerAdvancePreviousStateByte1WhenHelperReadyBossBoss05016_800C1A84(
    void) {
  u8 *previous;

  previous = g_battle_work;
  g_battle_work = *(u8 **)(D_801EC2E0 + 0x74);
  if (func_8014DAEC()) {
    previous[1] = previous[1] + 1;
  }
  g_battle_work = previous;
}
