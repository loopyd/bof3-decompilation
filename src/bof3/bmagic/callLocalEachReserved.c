#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work and declared here the way this target's other lifts declare
 * it. This body read it once for the pair selector, saved it, swapped it to a
 * pool record around the per-record call and restored the saved value, so each
 * access materialises the cell's own address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer cell in main RAM at 0x801EB4E0, outside this
 * overlay image, so the generated undefined_syms_auto.txt binds it. A pool
 * record's word 0x74 holds the work pointer that record activation saved: the
 * sibling
 * claimRecordClearByte1StoreWorkPointerPublishSavedPointerSetWorkByteBToOneAndAdvanceHandlerIndexBmagicMagic08103_801EEEDC
 * writes that word and the sibling
 * publishSavedWorkPointerInvokeReadinessHelperThenForwardBattleByteRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic08103_801EEF88
 * takes it back out through this same kind of cell. This body swaps the cell to
 * each record's word 0x74 around its handler call. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* The action-slot pool at 0x801F0F1C, stride 0x78, and the word 0x74 view of the
 * same records at 0x801F0F90. The sibling
 * resourceReserveFreeRecord5 proves the pool base, the
 * stride and the flags bit this body tests, and 24 records of stride 0x78 fill
 * the pool exactly to the end of this payload. */
extern u8 D_801F0F1C[]; /* @source 0x801F0F1C @kind data */
extern u8 D_801F0F90[]; /* @source 0x801F0F90 @kind data */

/* The two work-byte-1 entry handlers of this overlay, both still `asm` rows of
 * this target's Splat. Entry 0 clears bytes 0x0, 0x1 and 0x2 of every pool
 * record; entry 1 sets bit 0x04 of the halfword at 0x801462E8 when work byte
 * 0xB reads zero. */
extern void stateClearPoolBytes(void); /* @source 0x801EED04 @kind function */
extern void raiseFlag4AtZero(void); /* @source 0x801EF080 @kind function */

/* Work-byte-1 dispatcher over this overlay's local handler table at
 * 0x801F0F04, emitted by magic08303DispatchTables.c. */
extern void
callStateTableDispatch(void);

/* @source 0x801EEC04
 * @behavior Overlay entry step: reads the scratchpad work-object pointer cell
 *           at 0x1F800044 once, seeds 0x801EED04 into entry 0 and 0x801EF080
 *           into entry 1 of a two-entry local handler array in its own frame and
 *           calls the entry selected by the work object's unsigned byte 0x01
 *           with no arguments. It then walks all 24 records of the pool at
 *           0x801F0F1C with a byte counter, and for each record whose flags byte
 *           has bit 0 set it saves the work cell and the shared battle record
 *           cell at 0x801EB4E0, publishes the record pointer into the work cell
 *           and the record's word 0x74 pointer into 0x801EB4E0, calls the
 *           work-byte-1 dispatcher over 0x801F0F04 and restores both cells in
 *           reverse order. Nothing is returned and no argument is passed, so the
 *           overlay loader is the only caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callLocalEachReserved(
    void) {
  void (*handlers[2])(void) = {stateClearPoolBytes, raiseFlag4AtZero};
  u8 index;
  u8 flags;
  u32 offset;
  u8 *record;
  u8 *saved_work;
  u8 *saved_record;
  u8 *saved_word;

  handlers[g_battle_work[1]]();
  for (index = 0; index < 0x18; index++) {
    offset = index * 0x78;
    flags = D_801F0F1C[offset];
    if (flags & 1) {
      record = &D_801F0F1C[offset];
      saved_work = g_battle_work;
      saved_record = D_801EB4E0;
      saved_word = *(u8 **)(D_801F0F90 + offset);
      g_battle_work = record;
      D_801EB4E0 = saved_word;
      callStateTableDispatch();
      D_801EB4E0 = saved_record;
      g_battle_work = saved_work;
    }
  }
}
