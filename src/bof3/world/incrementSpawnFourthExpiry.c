#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 before the frame is created, then
 * again for the countdown tail and once more for the state-byte advance, so the
 * cell is read through this target's map name rather than the constant-address
 * accessor, whose shared base would stay live across the calls in a saved
 * register and widen the frame. */
extern u8* scratchWorkPointerAt1; /* @source 0x1F800044 @kind data */

/* Free-record scan of the shared 0x1C-byte-stride work table at 0x800E4800:
 * walks the sixteen records, returns the first one whose lead byte reads zero,
 * and returns null once every record is taken. */
u8* findFreeWorkRecordWorld02Area09813_801F4D80(void);

/* Record spawn: takes the claimed record and seeds its lead byte with 1, its
 * handler index, its countdown byte at +0x02 with 8, its delay byte at +0x04
 * with 0x20, its word at +0x08 with zero and its position words at +0x0C, +0x10
 * and +0x14 from the work record's triple at +0x34 offset by two rand-scaled
 * terms. */
void func_801F4DBC(u8* record);

/* Per-frame record pass over the same 0x800E4800 table: dispatches each live
 * record's own handler and returns the count of records it ran, which the next
 * handler of this row tests for zero. */
u8 func_801F4E7C(void);

/* @source 0x801F3EB0
 * @behavior Overlay progress step handler, entry 1 of the four-word handler row
 *           based at 0x801F5FAC. It increments the per-phase frame counter
 *           halfword at +0x30 of the work record published at the scratchpad
 *           pointer cell 0x1F800044, and whenever the low two bits of that
 *           record's countdown halfword at +0x2E read clear it claims a free
 *           record through 0x801F4D80 and, when that scan returns a record,
 *           spawns it through 0x801F4DBC. It then runs the per-frame record pass
 *           0x801F4E7C, re-reads the work record through the cell, decrements
 *           the countdown halfword at +0x2E and, on the frame that countdown
 *           reaches zero, clears the record byte at +0x09 and advances the state
 *           byte at +0x01, which hands the record to the next handler of the
 *           row. It takes no arguments, returns nothing, and its 0x18-byte frame
 *           only holds $ra at 0x10 across the three calls.
 *
 *           Dispatch evidence: the payload words 0x801F3E64, 0x801F3EB0,
 *           0x801F3F50 and 0x801F400C are the four entries of the handler row
 *           based at 0x801F5FAC (the row ends where the three-word handler row
 *           at 0x801F5FBC begins), and the exact
 *           dispatchStateByte1Table5facWorld02Area09813_801F3E20 scales the work
 *           record's state byte at +0x01 by four onto that base and jalrs the
 *           selected entry, so this span is the state-1 progress handler of that
 *           row and its caller stays outside the payload. No jal immediate names
 *           this address, so the absence of arguments is structural rather than
 *           declared.
 *
 *           Naming evidence: entry 0 of the same row, the
 *           statePhaseFrameCounter
 *           lift, arms the two halfwords this body consumes - it stores 0x40 at
 *           +0x2E, stores zero at +0x30 and then advances the state byte - so
 *           +0x2E is the phase countdown in frames and +0x30 the phase frame
 *           counter.
 *           Entries 2 and 3, func_801F3F50 and func_801F400C, keep the same two
 *           halfwords as phase timer and fade counter, the latter with the same
 *           decrement, store, shift-and-branch countdown tail this span carries
 *           at the same offset. The record byte at +0x09 counts down in the
 *           exact sibling claimPassesIntegrateReload,
 *           which this handler clears when its own frame countdown expires; the
 *           state-byte advance on expiry is that same sibling's closing step.
 *
 *           Shape evidence: the 160-byte span is not byte-identical to any span
 *           of the corpus - a scan of every payload under out/binaries for its
 *           first 24 bytes found only this copy, and its distinguishing pair of
 *           halfword loads at +0x30 and +0x2E occurs nowhere else - so no clone
 *           applied and no near twin supplied it. The statement order of the
 *           cadence block (counter increment first, so its store fills the branch
 *           delay slot, then the low-two-bits test on the phase countdown), the
 *           null check around the spawn and the named-cell reload before the
 *           countdown tail follow the exact 3BD4 lift of this target; the
 *           halfword decrement, store and zero test follow the byte-identical
 *           form of func_801F400C's tail. The spawn's triple-position seeds
 *           belong to the still-anchored callee and are read only to describe
 *           that call's role here.
 *
 *           Representation notes: the cell is spelled as the target's map name
 *           and read three times, once before the frame for the cadence block,
 *           once after the record pass for the countdown tail and once for the
 *           state byte, which is the three lui/lw pairs the original carries.
 *           Four spellings of the two blocks were measured. Reusing the cadence
 *           pointer variable for the countdown tail, with the decrement tested
 *           directly on the field, measured 27/40 instructions with the first
 *           difference at +0x0000 (the allocator placed the cell pointer in $a0
 *           where the original uses $v0 for it and $v1 for the tail); caching the
 *           countdown load above the counter increment, or holding the
 *           incremented counter in its own halfword variable, measured 27/40
 *           with the same first difference. Giving the tail its own pointer
 *           variable fixed the allocation but left the expiry test at 39/40 with
 *           the first difference at +0x0064, where the narrowing compiled to
 *           andi v0,v0,0xffff instead of the original sll v0,v0,0x10. Holding
 *           the decremented countdown in a signed-short local produced that sll
 *           and measured 40/40 over 160 bytes with no difference; the same
 *           cast applied inline to the field stayed at 39/40, so the local is
 *           the shape the compiler followed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementSpawnFourthExpiry(
    void) {
  u8* work;
  u8* record;
  u8* tail;
  s16 remaining;

  work = scratchWorkPointerAt1;
  *(u16*)(work + 0x30) += 1;
  if ((*(u16*)(work + 0x2E) & 3) == 0) {
    record = func_801F4D80();
    if (record != 0) {
      func_801F4DBC(record);
    }
  }
  func_801F4E7C();
  tail = scratchWorkPointerAt1;
  remaining = --*(u16*)(tail + 0x2E);
  if (remaining == 0) {
    tail[9] = 0;
    scratchWorkPointerAt1[1]++;
  }
}
