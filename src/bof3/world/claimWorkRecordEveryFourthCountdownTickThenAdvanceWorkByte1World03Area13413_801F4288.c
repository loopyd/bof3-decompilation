#include "bof3/bof3.h"

void claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0(void);
u32 func_801F49A0(void);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for each of the three store groups below. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F4288
 * @behavior Overlay state handler: entry 1 (work state byte 1 = 1) of the
 * handler table based at 0x801F581C that this target's dispatcher
 * dispatchStateByte1Table581cWorld03Area13413_801F41D0 indexes with the work
 * record's byte at +1, reached the frame after the run's first-step
 * initialiser 0x801F4214 sets that byte. It reads the countdown byte at offset
 * 0x09 of the work record published at the scratchpad pointer cell 0x1F800044
 * and, while its low two bits are clear - every fourth tick of the 0x80 the
 * initialiser armed - calls the work-record spawn at 0x801F48B0, which seeds
 * the first free entry of the 0x28-byte-stride record table at 0x800E4800
 * from that work record. It then decrements the countdown byte modulo 256 and,
 * when the decremented byte reads zero, advances the record's work byte at
 * offset 0x01 by one so the next frame dispatches entry 2 at 0x801F4310. The
 * handler always ends with the 0x800E4800 table walk at 0x801F49A0, whose
 * result it discards. Takes no arguments and returns nothing; its 0x18-byte
 * frame only holds $ra at 0x10 across the two calls, whose delay slots stay
 * nop. The pointer cell is addressed and reloaded again for each of the three
 * store groups, which the fixed-address cell symbol preserves.
 *
 * Name evidence: the only in-image reference to this address is its word in
 * the handler table based at 0x801F581C, dispatched through work byte 1 by
 * dispatchStateByte1Table581cWorld03Area13413_801F41D0, whose entry 0 at
 * 0x801F4214 is this target's exact
 * clearWorkTableOnFirstStepArmCountdownStampStepCountAndAdvanceWorkByte1World03Area13413_801F4214
 * and records that it arms the countdown byte 0x09 with 0x80 and advances the
 * work byte 1 so the next frame dispatches this entry. The claim target's role
 * is this target's exact lift
 * claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0, whose own name
 * evidence records this address as its only caller and reads the same
 * (published work byte 9 & 3) == 0 cadence as the every-fourth-tick condition
 * of that 0x80 countdown. The tail callee 0x801F49A0 is still anchored to its
 * address; this target's exact
 * clearWorkFlagsWhenNoCallbackRanWorld03Area13413_801F4310 records it as the
 * walk over the 0x20 entries of the 0x28-byte-stride work table at 0x800E4800
 * that runs each selected entry's handler and leaves 1 in its return register
 * once at least one entry ran one, so its result is consumed as a status there
 * and discarded here. The record field roles stay described by offset.
 *
 * Shape evidence: the countdown group follows the statement order and the u8
 * accumulator spelling of the scenario overlay's exact
 * tickCountdownByte9Advance12BitWorkWord0CAndStateScenarioScena0800_801F6FC4 -
 * `countdown = (u8)(work[9] - 1); work[9] = countdown;` tested for zero, with
 * the byte-1 advance read again through the cell symbol - and this target's
 * own literals, gate operand and reload points come from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimWorkRecordEveryFourthCountdownTickThenAdvanceWorkByte1World03Area13413_801F4288(
    void) {
  u8* work;
  u8 countdown;

  if ((D_1F800044[9] & 3) == 0) {
    claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0();
  }
  work = D_1F800044;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    D_1F800044[1]++;
  }
  func_801F49A0();
}
