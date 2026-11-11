#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); spelling the
 * bare constant address makes the compiler keep one base in a saved register
 * across the two calls instead, which costs the frame slot.
 * @source 0x1F800044 @kind data
 */
extern u8* workRecordCursor1F800044;

/* Flag bank of the shared main executable that the shared flag tester below
 * reads. This body passes its address, so it is declared as the object the
 * address names.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;

/* Shared small mode byte of the main executable. The neighbouring handlers at
 * 0x801F4B40 and 0x801F48DC treat the same byte as the overlay's mode, this
 * target's dispatch-state handler 0x801F4864 steps it, and this body only loads
 * it with `lbu` as a nonzero gate, so its role beyond that count is not
 * established here and the kind stays unknown.
 * @source 0x8014686B @kind unknown
 */
extern u8 D_8014686B;

/* Shared flag tester of the main executable: reads the bit its second argument
 * selects in the flag bank its first argument addresses and reports it as a
 * nonzero return value.
 * @source 0x8015B5D4
 */
extern s32 func_8015B5D4(u32 flag_bank, s32 bit_index);

/* Shared frontend cue dispatcher of the main executable at 0x8015DF18; the cue
 * id arrives zero-extended in the whole argument word.
 * @source 0x8015DF18
 */
extern void func_8015DF18(u16 cue);

/* @source 0x801F3E38
 * @behavior Entry 1, the second of the six handlers of the byte-+0x02 run based
 *           at 0x801F4E54 that this target's dispatcher at 0x801F3D60 selects,
 *           so no jal reaches this body. It reads the scratchpad work-record
 *           pointer cell 0x1F800044 and tests the record's signed halfword at
 *           +0x36, the grid row the move-block emitter of this run is handed.
 *           While that halfword is zero it asks the shared flag tester
 *           0x8015B5D4 for bit 0xD of the main-RAM flag bank 0x80144F28; while
 *           the halfword is nonzero it re-reads the cell, re-reads the same
 *           halfword and then loads the shared mode byte 0x8014686B with
 *           `lbu`. When either of those two gates holds - a zero halfword with
 *           that flag bit set, or a nonzero halfword with a nonzero mode byte -
 *           it requests frontend cue 0x20D through the shared frontend cue
 *           dispatcher 0x8015DF18, re-reads the cell and clears work byte
 *           +0x09, then re-reads the cell and advances the dispatch state byte
 *           +0x02 by one, which selects entry 2 of the same run for the next
 *           frame. In every other case it returns without touching the record.
 *           The 0x18-byte frame holds the return address at +0x10; the function
 *           takes no arguments and returns nothing.
 *
 * Name evidence: the handler table at 0x801F4E54 holds this span's address as
 *   its word 1, between the already-named entries 0x801F3DA4 and 0x801F3EE4,
 *   and the dispatcher lifted at 0x801F3D60 indexes that table with the work
 *   record's byte +0x02, so the byte this body advances is the run's dispatch
 *   state and the value it stores selects the run entry. The posted id is the
 *   0x20D immediate the body loads into the cue dispatcher's argument word, the
 *   same cue the exact handler 0x801F4864 posts through the same dispatcher.
 *   The flag probe is bit 0xD of bank 0x80144F28, the bit the exact handler
 *   0x801F3DA4 tests with the same tester and the overlay's 0x801F4804 handler
 *   sets through the shared setter. The halfword +0x36 is the grid row that the
 *   exact handlers 0x801F40B4 and 0x801F4148 hand to the emitter 0x801F41F0,
 *   and byte +0x09 is the scroll column the exact handlers 0x801F3874,
 *   0x801F40B4 and 0x801F4148 step.
 *
 *   The caller is unnamed: no `jal` in any of the 396 shipped payloads targets
 *   this address, and no word of any payload equals 0x801F3E38 outside the
 *   handler table above, so the body takes no arguments and its roles stay
 *   structural.
 *
 * Shape evidence: the unlifted neighbour 0x801F3F98 shows this exact gate in
 *   its own bytes - the same hoisted cell load above the frame creation, the
 *   same bit-0xD probe through the same tester, the same re-read of halfword
 *   +0x36 and the same `lbu` of the mode byte 0x8014686B, in the same order and
 *   at the same join points. It is the same gate with the opposite polarity
 *   (its two tests are `beqz`/`bnez` where this span's are `bnez`/`beqz` and
 *   `beqz`/`bnez`), so this span's operators are written from its own branch
 *   words. The join block's `sb $zero,0x9` and `lbu 0x2 / addiu 1 / sb 0x2`
 *   pair is 0x801F3F98's own block, which this span prefixes with the cue call.
 *   The per-access pointer spelling of the scratchpad cell and the frame come
 *   from the exact sibling 0x801F3DA4, the only difference being the gate
 *   arm contents. Every literal here is written from this span's own bytes:
 *   flag bit 0xD, cue id 0x20D, state step 1.
 *
 * Clone declined: one scan of all 396 extracted payloads under out/binaries for
 *   this 0xAC-byte body, and a rescan of every 4-byte aligned offset with each
 *   `jal`/`j` immediate word masked so a relocation-different sibling could
 *   still match, returned only this span's own payload offset 4664. The nearest
 *   other shape is 0x801F3F98 in this payload, whose gate is inverted and whose
 *   join block makes one call fewer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue20DThenClearWorkByte9AndIncrementState2WhenWorkHalfword36IsZeroWithFlagDSetOrNonzeroWithModeByteSetWorld01Area04813_801F3E38(
    void) {
  if ((FIELD_REF(s16, workRecordCursor1F800044, 0x36) == 0 &&
       func_8015B5D4((u32)&D_80144F28, 0xD) != 0) ||
      (FIELD_REF(s16, workRecordCursor1F800044, 0x36) != 0 &&
       D_8014686B != 0)) {
    func_8015DF18(0x20D);
    workRecordCursor1F800044[0x09] = 0;
    workRecordCursor1F800044[0x02] += 1;
  }
}
