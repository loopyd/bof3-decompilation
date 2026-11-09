#include "base/types.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The body reads the cell once before publishing the saved
 * pointer and stores the saved value back once after the helper call, and the
 * original re-materialises the cell address with its own lui for each of the
 * two stores, so the cell is reached through the named pointer rather than a
 * folded constant base. The plain declaration is the one the exact siblings
 * decrementGlobalWorkByteBAndClearWorkBytes0Through4BmagicMagic08103_801EF614
 * and reserveRecordEveryFourthTickPublishWorkPointerCopyWorkByte9SeedByteAStep
 * ThenAdvanceWorkByteBAndByte9AndByte2AtLeast81BmagicMagic08103_801EF18C use. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. Word 0x74 of
 * that record is the saved work-object pointer: the sibling
 * reserveRecordEveryFourthTickPublishWorkPointerCopyWorkByte9SeedByteAStepThen
 * AdvanceWorkByteBAndByte9AndByte2AtLeast81BmagicMagic08103_801EF18C writes the
 * work object it publishes into exactly that word, and this function takes it
 * back out. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Readiness poll helper of the shared EXE image at 0x8014DAEC, outside this
 * overlay's image. The battle, bmagic, plchar and ui lifts declare the same
 * zero-extended byte prototype, and Splat binds the address through this
 * target's generated undefined_funcs_auto.txt. Only the returned byte is tested
 * here, as the bmagic siblings advanceWorkByte2WhenHelperReadyBmagicMagic00800
 * 801F117C and setWorkByte9To15AndAdvanceWorkByte2WhenHelperReadyBmagicMagic
 * 13103_801EFEF0 do, so its role stays address-anchored. */
extern u8 func_8014DAEC(void);

/* @source 0x801EEE74
 * @behavior Republishes the battle record's saved work-object pointer through
 *           the scratchpad work-object pointer cell at 0x1F800044 and, when the
 *           shared readiness helper reports ready, advances the work handler
 *           index of the record that was published before this call. It reads
 *           the shared battle record pointer D_801EC2E0, takes that record's
 *           word 0x74 - the saved work-object pointer - and stores it at the
 *           cell, keeping the cell's previous value in a callee-saved register.
 *           It then calls func_8014DAEC and tests only the zero-extended low
 *           byte of the result: on zero it writes nothing more, on non-zero it
 *           increments byte 1 of the previously published work object through
 *           the save register. Byte 1 of the scratchpad work object is the
 *           handler index this overlay dispatches on - the sibling
 *           incrementWorkByte1WhenWorkByteBIs1BmagicMagic04103_801EEFC4 records
 *           that role for the same cell, and this target's own dispatch entry
 *           dispatchStateByte1Tablefa84BmagicMagic08103_801EF014 loads that
 *           byte and indexes the pointer run at 0x801EFA84 with it - so the
 *           name advances a handler index rather than an anonymous counter.
 *           The cell is then restored to the saved pointer, so the helper and
 *           the increment both run while the record's own saved pointer is
 *           published. Takes no arguments and returns nothing; the 0x18-byte
 *           frame only keeps ra at 0x14 and the saved cell value at 0x10 across
 *           the one call, and no other state is read or written. Nothing in
 *           this overlay names word 0x74 for the record it belongs to, so the
 *           name reports the publish-and-restore effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishSavedWorkPointerAdvancePreviousHandlerIndexWhenHelperReadyBmagicMagic08103_801EEE74(
    void) {
  u8 *saved;

  saved = g_battle_work;
  g_battle_work = *(u8 **)(D_801EC2E0 + 0x74);
  if (func_8014DAEC()) {
    saved[1]++;
  }
  g_battle_work = saved;
}
