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
 * 0x801F4B40 and 0x801F48DC treat the same byte as the overlay's mode and this
 * target's dispatch-state handler 0x801F4864 steps it, while this body only
 * loads it with `lbu` as a zero gate, so its role beyond that count is not
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

/* Per-row draw routine at 0x801F4258, unlifted. Its own bytes index the byte
 * tables at 0x801F4E78, 0x801F4E7C, 0x801F4E80 and 0x801F4E84 with the grid row
 * its first argument carries, then submit that row's primitives through
 * RotTransPers, SetDrawMode and SetSemiTrans, so the argument stays a row index
 * by construction and nothing else is claimed about the body.
 * @source 0x801F4258
 */
extern void func_801F4258(s16 row);

/* PsyQ libc random source of the shared main executable, outside this payload.
 * The body keeps only its low bit and uses it to decide one dispatch-state
 * store.
 */
extern int rand(void);

/* @source 0x801F3F98
 * @behavior Overlay step handler: entry 3 of the six-handler run based at
 *           0x801F4E54 that this target's byte-+0x02 dispatcher at 0x801F3D60
 *           selects, so no jal reaches this body. It first gates two ways on
 *           the scratchpad work-record pointer cell 0x1F800044: with the
 *           record's signed halfword +0x36 zero it asks the shared flag tester
 *           0x8015B5D4 whether bit 0xD of the shared main-RAM flag bank
 *           0x80144F28 reads clear, and with that halfword nonzero it tests the
 *           shared mode byte 0x8014686B. While either gate holds it clears the
 *           record's work byte +0x09 and advances its dispatch state byte +0x02
 *           by one, which selects entry 4 of the same run for the next frame.
 *           Unlike the two copy-over state entries that bracket it, this body
 *           draws nothing itself in that arm; it then runs the same tail on
 *           every call, advancing work byte +0x09 and, once that byte reaches
 *           0x41, clearing it and, when the low bit of rand() is set,
 *           overwriting the dispatch state byte +0x02 with 5. A last cell read
 *           passes the record's halfword +0x36 to the per-row draw routine at
 *           0x801F4258. The 0x18-byte frame holds the return address at +0x10;
 *           the function takes no arguments and returns nothing.
 *
 *           Name evidence: the sibling entry-1 handler lifted at 0x801F3E38
 *           carries the same two-gate shape with the opposite polarity
 *           ("...WhenWorkHalfword36IsZeroWithFlagDSetOrNonzeroWithModeByteSet"),
 *           and its own comment records that this span holds the inverted gate
 *           with the same join block and one call fewer, so the name follows
 *           that sibling with this body's polarity ("FlagDClear",
 *           "ModeClear") and this body's tail ("AdvanceWorkByte9",
 *           "WrapAt41", "RandomState5", "DrawRow"). The 0x41 bound and the 5
 *           it stores are written from this span's own bytes, where the
 *           neighbouring wrap-at-0x41 handler at 0x801F3874 names the same
 *           hex literal in the same way. Roles this span does not establish
 *           stay structural: the flag tester's two arguments, the row routine's
 *           single argument and the meaning of the mode byte beyond its zero
 *           gate.
 *
 *           Shape evidence: the statement order, the pointer-local spelling for
 *           the scratchpad cell, the two-gate `||` of two `&&` pairs that
 *           re-reads the cell's halfword, the join block's clear-then-advance
 *           pair and the frame come from the exact sibling at 0x801F3E38 of
 *           this payload, whose gate is the inversion written above; the
 *           advance, the `>= ` bound, the clear-then-reload arm and the
 *           trailing row call come from the unlifted twin at 0x801F3EE4 and
 *           from the exact sibling at 0x801F4148, whose own bytes and body
 *           show that shape. Every literal here is this span's own: flag bit
 *           0xD, bound 0x41, the random low-bit test and the state 5.
 *
 *           Clone was declined on measured evidence: one scan of all 396
 *           extracted payloads under out/binaries for this span's 0x11C-byte
 *           body found it only at this payload's own offset 0x1398, so no
 *           byte-identical twin exists to clone and the in-payload twins differ
 *           at the gate polarity, the bound and the tail, which is shape
 *           evidence rather than clone proof. The data lever was declined by
 *           this target's empty data-candidate list and the carve lever by a
 *           span that opens on its own prologue and ends at the next declared
 *           boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9AndIncrementState2WhenWorkHalfword36IsZeroWithFlagDClearOrNonzeroWithModeClearThenAdvanceWorkByte9WrapAt41ToRandomState5AndDrawRowWorld01Area04813_801F3F98(
    void) {
  u8* clearWork;
  u8* stepWork;
  u8* incWork;
  u8* gateWork;
  u8* resetWork;
  u8* rowWork;

  if ((FIELD_REF(s16, workRecordCursor1F800044, 0x36) == 0 &&
       func_8015B5D4((u32)&D_80144F28, 0xD) == 0) ||
      (FIELD_REF(s16, workRecordCursor1F800044, 0x36) != 0 &&
       D_8014686B == 0)) {
    clearWork = workRecordCursor1F800044;
    clearWork[0x09] = 0;
    stepWork = workRecordCursor1F800044;
    stepWork[0x02] += 1;
  }
  incWork = workRecordCursor1F800044;
  incWork[0x09] += 1;
  gateWork = workRecordCursor1F800044;
  if (gateWork[0x09] >= 0x41u) {
    gateWork[0x09] = 0;
    if (rand() & 1) {
      resetWork = workRecordCursor1F800044;
      resetWork[0x02] = 5;
    }
  }
  rowWork = workRecordCursor1F800044;
  func_801F4258(FIELD_REF(s16, rowWork, 0x36));
}
