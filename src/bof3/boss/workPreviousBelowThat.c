#include "bof3/bof3.h"

extern u8 *boss028CurrentRecordState; /* @source 0x1F800044 @kind data */

/* Shared main-program helper the whole boss archive family calls: it returns
 * the distance between the fixed-point position pair it takes as (x, z). Both
 * uses here add a slack to the returned word and compare or store the whole
 * word, so the return type is the full `s32` at this site. This target's
 * generated undefined_funcs_auto.txt already binds the address, so the callee
 * needs no map row. */
extern s32 func_8015477C(s32 arg0, s32 arg1);

/* Main-executable helper at 0x8015D71C, invoked with the selector 0x600. Its
 * answer is dead at this call site - the body reads work byte 9 next - so the
 * declaration carries no return value. Splat binds 0x8015D71C through this
 * target's generated undefined_funcs_auto.txt, so no target-map row is needed. */
extern void func_8015D71C(s32 arg0);

/* @source 0x800C1D90
 * @behavior Per-frame arrival entry of this payload's seven-slot work-state run:
 *           it loads the work record pointer from the scratchpad pointer cell
 *           0x1F800044 once, ahead of its own frame setup, and adds the record's
 *           unsigned halfword 0x3E by its word 0x14 as that word stood before
 *           this update, then advances word 0x14 by word 0x20. The word 0x14 the
 *           halfword update consumes is the original's own second load of that
 *           address rather than a copy of the value the update stores, because
 *           the halfword statement precedes the update in the body and its store
 *           to 0x3E may alias the word. It re-reads halfword 0x3E as a signed
 *           value, takes the func_8015477C distance of the record's position
 *           word pair at 0x34 and 0x38, and runs the arrival block only while
 *           that accumulated halfword is still below that distance plus 0x200.
 *           The block samples the cell twice, once for the argument pair of its
 *           own distance call and again after that call for the store base, and
 *           that second sample serves every store of the block: it stores the
 *           freshly measured distance plus 0x200 back into halfword 0x3E, clears
 *           word 0x14, seeds word 0x20 with the signed 0x40 and invokes
 *           func_8015D71C with the selector 0x600. It then tests work byte 9, the
 *           countdown entry 1 of the byte-2 run decrements on the way to the
 *           effect submit: while that countdown is still nonzero it decrements
 *           the byte and advances work byte 2 by one, and once the countdown has
 *           reached zero it advances work byte 2 by two instead. Both arms reach
 *           the one byte-2 store, which hands the byte-2 dispatcher to its next
 *           slot. The 0x18-byte frame keeps $ra and $s0 across the guard call,
 *           and $s0 holds the signed halfword the comparison still needs after
 *           it returns. The function takes no arguments, returns nothing and
 *           writes no state outside the work record.
 *
 *           No jal names this address. The payload word at 0x800C21B0 holds it
 *           as slot 1 of the seven-slot code pointer run based at 0x800C21AC that
 *           dispatchWorkByte2Table21acThenInvokeSharedServiceWhenWorkByte0Bit0Set
 *           BossBoss02816_800C1C30 indexes with work record byte 2, and slot 0 at
 *           0x800C1C9C ends by advancing that byte to reach this body, so the
 *           prologue and the return path place this boundary; the roles of a
 *           dispatched entry are structural and no caller prototype is owed.
 *
 *           Its first 0x50 bytes are instruction-for-instruction identical to
 *           the exact sibling addWorkWord20ToWord14AccumulatePreviousWord14Into
 *           Halfword3EThenStoreZeroInWord14AndMinus40InWord20DecrementWorkByte2
 *           WhenHalfword3EAboveRemeasuredDistancePlus400BossBoss02816_800C1E80,
 *           which carries the same integration prefix statement for statement
 *           and parts from this body at the comparison, the slack and the
 *           guarded block, and the exact mirror sibling
 *           addWorkWord20ToWord14AccumulatePreviousWord14IntoHalfword3EThen
 *           AdvanceWorkByte2WhileBelowDistancePlus180BossBoss05416_800C1FA8
 *           carries the same below-guard with its own slack and a block that only
 *           advances the step byte; the two of them supplied the statement order,
 *           the access widths, the double read of word 0x14, the named-cell
 *           reload of the block and the below-guard's operand order, and this
 *           body writes its own slack 0x200, its own block stores, its own
 *           selector 0x600 and its own countdown tail from its own bytes. A scan
 *           of the 398 extracted payloads under out/binaries matched this 0xF0-byte
 *           span only at its own offset, both exactly and with every jal and j
 *           immediate masked, and no payload matched it at opcode/register
 *           granularity with every immediate dropped, so no other payload carries
 *           this body and those siblings are shape evidence, not clone sources.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workPreviousBelowThat(
    void) {
  u8 *work;

  work = boss028CurrentRecordState;
  *(u16 *)(work + 0x3E) += *(u32 *)(work + 0x14);
  *(u32 *)(work + 0x14) += *(u32 *)(work + 0x20);
  if (*(s16 *)(work + 0x3E) <
      func_8015477C(*(s32 *)(work + 0x34), *(s32 *)(work + 0x38)) + 0x200) {
    *(u16 *)(boss028CurrentRecordState + 0x3E) =
        func_8015477C(*(s32 *)(boss028CurrentRecordState + 0x34), *(s32 *)(boss028CurrentRecordState + 0x38)) +
        0x200;
    *(u32 *)(boss028CurrentRecordState + 0x14) = 0;
    *(u32 *)(boss028CurrentRecordState + 0x20) = 0x40;
    func_8015D71C(0x600);
    if (boss028CurrentRecordState[9] != 0) {
      boss028CurrentRecordState[9]--;
      boss028CurrentRecordState[2]++;
    } else {
      boss028CurrentRecordState[2] += 2;
    }
  }
}
