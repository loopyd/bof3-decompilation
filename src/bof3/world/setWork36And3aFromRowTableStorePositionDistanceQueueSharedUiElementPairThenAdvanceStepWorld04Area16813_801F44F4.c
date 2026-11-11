#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell published by the front end. This body
 * addresses the record through the named cell at every access, because the
 * original loads it once per group: before the byte store, then again after
 * that byte store and after each of the three calls, and never keeps it in a
 * saved register.
 * @source 0x1F800044 @kind data
 */
extern u8* area16813WorkCursor_1F800044;

/* First column of the six-row stride-2 byte pair table of this payload's own
 * textbin tail at 0x801F4B2C: row k is the pair (0x801F4B7C[2k],
 * 0x801F4B7D[2k]) and the run ends with the zero pair. The body republishes row
 * k into the record halfwords +0x36 and +0x3A, with k taken from +0x36 itself.
 * @source 0x801F4B7C @kind data
 */
extern u8 D_801F4B7C[];
/* Second column of the same six-row stride-2 byte pair table.
 * @source 0x801F4B7D @kind data
 */
extern u8 D_801F4B7D[];

/* Unsigned halfword row table of the same tail: row k is the term the body
 * subtracts from the halved distance before storing record halfword +0x3E.
 * @source 0x801F4B5C @kind data
 */
extern u16 D_801F4B5C[];

/* Word row table of the same tail: row k is the value the body stores into
 * record word +0x20.
 * @source 0x801F4B68 @kind data
 */
extern u32 D_801F4B68[];

/* Word row table of the same tail: row k is the value the body truncates into
 * record byte +0x0B.
 * @source 0x801F4BC8 @kind data
 */
extern u32 D_801F4BC8[];

/* Word row table of the same tail: row k is the size both shared world-marker
 * queue calls pass.
 * @source 0x801F4BDC @kind data
 */
extern u32 D_801F4BDC[];

/* Distance helper of the shared main executable: it takes the record word pair
 * at +0x34 and +0x38 and answers a signed halfword, which the original
 * sign-extends with the `sll`/`sra` pair before negating it.
 * @source 0x8015477C
 */
extern s16 func_8015477C(s32 x, s32 y);

/* Shared world-marker queue of the shared main executable; the sibling handlers
 * of this payload hand it a halfword pair and a size.
 * @source 0x801A4BC0
 */
extern void func_801A4BC0(s32 x, s32 y, u32 size);

/* Step helper of this payload, still an `asm` span at 0x801F3914 at the time of
 * this lift: the handlers of every state-byte-2 run in this payload end in it,
 * and it takes no arguments and answers nothing.
 * @source 0x801F3914
 */
extern void func_801F3914(void);

/* @source 0x801F44F4
 * @behavior Entry 0 of the state-byte-2 handler run based at 0x801F4BB8 of this
 *           payload, whose dispatcher is 0x801F44B0: the body address appears
 *           once in the payload bytes, at word 0x801F4BB8, and no `jal` in the
 *           payload names it, so it takes no arguments and answers no value. The
 *           body reads the work record through the scratchpad pointer cell
 *           0x1F800044 and keeps the previous +0x36 halfword as its row index.
 *           It stores the `sltiu`-folded test of the +0x3A halfword into record
 *           byte +0x08, then republishes halfword +0x36 from 0x801F4B7C[2k] and
 *           halfword +0x3A from 0x801F4B7D[2k], the two byte columns of one
 *           stride-2 row table. It hands the record word pair +0x34/+0x38 to the
 *           shared distance helper 0x8015477C, sign-extends the answer, negates
 *           it and halves it with the sign-fixup sequence, subtracts the
 *           unsigned word D_801F4B5C[k] and stores the halfword into +0x3E. It
 *           then stores word D_801F4B68[k] into +0x20, clears halfword +0x30 and
 *           the low byte of D_801F4BC8[k] into record byte +0x0B. It queues two
 *           shared world markers through 0x801A4BC0 at the freshly published
 *           (+0x36, +0x3A) and (+0x36, +0x3A + 1) with the word D_801F4BDC[k] as
 *           the size, increments record byte +0x02 so the next dispatch takes
 *           entry 1 of the same run, and calls the payload step helper
 *           0x801F3914 on every call. The 0x20-byte frame only keeps $ra, $s1
 *           and $s0, the first pointer load runs before the frame is created,
 *           and the cell is re-read after the byte store and after each of the
 *           three calls.
 *
 * Name evidence: 0x801F4BB8 is the entry run of the byte-2 dispatcher
 * 0x801F44B0 and its word 0 is this body, so the handler is the first
 * state-byte-2 handler of that run, and its +0x02 increment walks the run. The
 * record halfwords +0x36 and +0x3A are the halfword pair this payload's exact
 * sources leave address-anchored while the sibling handler 0x801F4634, entry 1
 * of the same run, passes them unchanged to the shared marker queue, and
 * 0x8015477C is the distance helper the plchar and world families declare as a
 * signed distance over a coordinate pair. The record words +0x34 and +0x38 are
 * the fixed-point position pair of that same scratchpad record, and the exact
 * world02/area103/13 source
 * storeWorkPositionDistanceAt3eWorld02Area10313_801F57FC hands the pair to the
 * same helper and names the value it stores a work position distance, which is
 * the vocabulary this name uses for the term derived here. 0x801F4B7C and
 * 0x801F4B7D are the stride-2 byte columns of one row table in this payload's
 * own textbin tail, which the exact sibling source
 * setStatusFFFFAndStateEntryAProgress5SubstateFFWhenGate3ThenPublishRecordFromGateRowWorld04Area16813_801F4764
 * reads the same way, and 0x801A4BC0 is the shared world-marker queue the exact
 * world04 sibling sources name that way. The roles of the +0x30, +0x3E, +0x20
 * and +0x0B record fields, and of every table row, stay address-anchored.
 *
 * Shape evidence: the in-target `asm` sibling 0x801F3424, entry 0 of the
 * neighbouring state-byte-2 run based at 0x801F4B88, repeats this prefix byte
 * for byte from its own 0x824 up to its 0x8E4: the same pre-frame pointer load,
 * the same `lh`/`lh`/`sltiu`/`sll`/`sb` group at +0x08, both byte-column
 * republishes, the distance call with the `sll`/`sra`/`negu`/sign-fixup
 * sequence and its payload-local `sll` in the delay slot, and the +0x3E store,
 * after which it diverges into the shared camera-word window tests. The next
 * span 0x801F4634, entry 1 of this run, queues the same two markers from the
 * record halfwords with a literal size 0xA1 and ends in the same step helper.
 * No byte-identical twin exists: the span's discriminating immediates are
 * payload-local table words, and the three overlay spans that also call both
 * 0x8015477C and 0x801A4BC0 (world02/area102/13 0x801F2C48, scena08/00
 * 0x801FA3E4 and etc/game/00 0x801A7330) carry different payload bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWork36And3aFromRowTableStorePositionDistanceQueueSharedUiElementPairThenAdvanceStepWorld04Area16813_801F44F4(
    void) {
  s32 x;
  s32 y;
  s16 distance;

  y = FIELD_REF(s16, area16813WorkCursor_1F800044, 0x3A);
  x = FIELD_REF(s16, area16813WorkCursor_1F800044, 0x36);
  area16813WorkCursor_1F800044[8] = (y == 0);
  FIELD_REF(u16, area16813WorkCursor_1F800044, 0x36) = D_801F4B7C[x * 2];
  FIELD_REF(u16, area16813WorkCursor_1F800044, 0x3A) = D_801F4B7D[x * 2];
  distance = func_8015477C(FIELD_REF(u32, area16813WorkCursor_1F800044, 0x34),
                           FIELD_REF(u32, area16813WorkCursor_1F800044, 0x38));
  FIELD_REF(u16, area16813WorkCursor_1F800044, 0x3E) =
      -distance / 2 - D_801F4B5C[x];
  FIELD_REF(u32, area16813WorkCursor_1F800044, 0x20) = D_801F4B68[x];
  FIELD_REF(u16, area16813WorkCursor_1F800044, 0x30) = 0;
  area16813WorkCursor_1F800044[0xB] = D_801F4BC8[x];
  func_801A4BC0(FIELD_REF(s16, area16813WorkCursor_1F800044, 0x36),
                FIELD_REF(s16, area16813WorkCursor_1F800044, 0x3A),
                D_801F4BDC[x]);
  func_801A4BC0(FIELD_REF(s16, area16813WorkCursor_1F800044, 0x36),
                FIELD_REF(s16, area16813WorkCursor_1F800044, 0x3A) + 1,
                D_801F4BDC[x]);
  area16813WorkCursor_1F800044[2]++;
  func_801F3914();
}
