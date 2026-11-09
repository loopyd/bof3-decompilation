#include "base/types.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work like the siblings of this target declare it. This body
 * takes three separate reads of the cell: one before the record writes, which
 * stays in a caller-saved register up to the record word 0x74 store, one after
 * that store for the value it keeps in a callee-saved register across the
 * helper call, and one after the cell is restored, because a store through the
 * loaded pointer may alias the cell and the original re-materialises the
 * address with its own lui / lw pair at each read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer in main RAM, outside this overlay image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row is added here. Word 0x74 of that record is the saved
 * work-object pointer: the sibling
 * reserveRecordEveryFourthTickPublishWorkPointerCopyWorkByte9SeedByteAStepThen
 * AdvanceWorkByteBAndByte9AndByte2AtLeast81BmagicMagic08103_801EF18C writes the
 * work object it publishes into exactly that word, and the sibling
 * publishSavedWorkPointerAdvancePreviousHandlerIndexWhenHelperReadyBmagicMagic0
 * 8103_801EEE74 takes it back out through this same pointer cell. */
extern u8 *D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */

/* Byte 1 and word 0x74 of the 0x78-byte-stride action-slot store record based at
 * 0x801EC330 in main RAM. The original reaches each field through its own flat
 * symbol - lui $at,%hi(SYM), addu $at,$at,index*0x78, store %lo(SYM)($at) - with
 * the scaled index (sll 4, subu, sll 3) in one register shared by both stores,
 * so every addressed field keeps its own address-anchored name instead of a
 * shared record base, exactly as the sibling lift
 * copyRecordWordPairAdd2000ToWord34Add100ToHalfword3EThenSetDirectionTowardSlot
 * RecordFromByteAStoreWord14ClearByte9AndAdvanceWorkByte2BmagicMagic08103_801EF
 * 39C declares the fields 0x801EC364 and 0x801EC368 of the same record. This
 * body clears byte 1, while the unfused sibling func_801EF0B8 sets it to 1, so
 * the role of that byte is not established beyond the value written here and
 * the address stays address-anchored. This target's generated
 * undefined_syms_auto.txt carries both addresses. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Action-slot allocator of the shared main executable at 0x801E590C, outside
 * this overlay image. The bmagic siblings of other targets declare the same
 * two-argument form and use the zero-extended low byte of the result as the
 * index of the record it claimed, with 0xFF as its no-free-entry answer:
 * queueSlotPerEligibleEnemyAndPartyBattlerSetByte1To2OrdinalByte4SelectorByteB
 * AndRecordPointerBmagicMagic06903_801F0274 records both facts. Bound by this
 * target's generated undefined_funcs_auto.txt row. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Readiness poll helper of the shared EXE image at 0x8014DAEC, outside this
 * overlay's image. The bmagic siblings declare the zero-extended byte
 * prototype; this body calls it for its effect and uses no part of the answer,
 * so its role stays address-anchored. */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC */

/* @source 0x801EEEDC
 * @behavior Word 2 of the four-word handler pointer run at 0x801EEC04 that the
 *           frame dispatcher func_801EEC30 copies to its frame and calls
 *           through work-object byte 1 scaled by four, so no jal in the payload
 *           names it. It claims one entry of the action-slot store through the
 *           shared allocator func_801E590C, called with selector 1 and 0x42, and
 *           takes the zero-extended low byte of the answer as the entry index.
 *           The index is scaled by the 0x78-byte record stride and reaches two
 *           fields of the claimed entry: record byte 1 is cleared, and record
 *           word 0x74 receives the scratchpad work-object pointer read from the
 *           cell at 0x1F800044. The body then republishes the shared battle
 *           record's saved work-object pointer, word 0x74 of the record reached
 *           through the pointer cell 0x801EC2E0, through the scratchpad cell
 *           while keeping the cell's previous value in a callee-saved register,
 *           calls the readiness helper func_8014DAEC, and restores the cell from
 *           that register. With the previous work object published again it sets
 *           work byte 0xB to 1 and increments work byte 1, the handler index this
 *           overlay dispatches on - the local dispatcher
 *           dispatchStateByte1Tablefa84BmagicMagic08103_801EF014 loads that byte
 *           and indexes the pointer run at 0x801EFA84 with it - so the next
 *           frame reaches word 3 of the run. Byte 0xB is the countdown the
 *           sibling decrementGlobalWorkByteBAndClearWorkBytes0Through4BmagicMag
 *           ic08103_801EF614 steps down and clears work bytes 0 through 4 for, so
 *           this body reloads it rather than merely advancing the index. Takes
 *           no arguments and returns nothing; the 0x18-byte frame only keeps ra
 *           at 0x14 and the saved cell value at 0x10 across the one call, and no
 *           other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimRecordClearByte1StoreWorkPointerPublishSavedPointerSetWorkByteBToOneAndAdvanceHandlerIndexBmagicMagic08103_801EEEDC(
    void) {
  u8 index;
  s32 offset;
  u8 *published;
  u8 *record;
  u8 *saved;

  index = (u8)func_801E590C(1, 0x42);
  offset = index * 0x78;
  published = g_battle_work;
  D_801EC331[offset] = 0;
  record = D_801EC2E0;
  saved = g_battle_work;
  *(u8 **)(D_801EC3A4 + offset) = published;
  g_battle_work = *(u8 **)(record + 0x74);
  func_8014DAEC();
  g_battle_work = saved;
  saved[0xB] = 1;
  g_battle_work[1]++;
}
