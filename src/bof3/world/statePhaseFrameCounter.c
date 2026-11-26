#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 after the reset call returns, so
 * the cell is read through this target's map name rather than a constant-address
 * accessor, whose shared base would stay live across the call in a saved
 * register and widen the frame. */
extern u8* scratchWorkPointerAt1; /* @source 0x1F800044 @kind data */

/* State-entry reset of the shared work table: clears the lead byte of each of
 * the 0x10 records of the 0x1C-byte-stride table at 0x800E4800 and zeroes the
 * two one-shot latch bytes at 0x801F6510 and 0x801F6514, re-arming the actions
 * their readers take on a first pass. */
void stateOneShotLatches(void);

/* Cue submission: takes the cue id in $a0 and returns nothing. Its body lies
 * outside this payload, so the argument role comes from the exact lifts of this
 * overlay engine that queue cue ids through it. */
void func_8015DF18(u16 cue);

/* @source 0x801F3E64
 * @behavior Overlay progress step handler, entry 0 of the four-word handler row
 *           based at 0x801F5FAC. It first calls the state-entry reset
 *           0x801F4D44, which clears the lead byte of each of the 0x10 records
 *           of the 0x1C-byte-stride work table at 0x800E4800 and zeroes the two
 *           one-shot latch bytes at 0x801F6510 and 0x801F6514. It then reads the
 *           work record through the scratchpad pointer cell 0x1F800044, keeps
 *           that record's state byte at +0x01, stores 0x40 into the phase
 *           countdown halfword at +0x2E, stores zero into the phase frame
 *           counter halfword at +0x30, writes the kept state byte plus one back
 *           to +0x01, which hands the record to entry 1 of the row, and queues
 *           cue 0x200 through 0x8015DF18. It takes no arguments, returns
 *           nothing, and its 0x18-byte frame holds only $ra at 0x10 across the
 *           two calls.
 *
 *           Dispatch evidence: the payload words 0x801F3E64, 0x801F3EB0,
 *           0x801F3F50 and 0x801F400C are the four entries of the handler row
 *           based at 0x801F5FAC (a read of this payload at that address; the row
 *           ends where the three-word handler row 0x801F5FBC begins), and the
 *           exact dispatchStateByte1Table5 scales
 *           the work record's state byte at +0x01 by four onto that base and
 *           jalrs the selected entry, so this span is the state-0 progress
 *           handler of that row and its caller stays outside the payload. No jal
 *           immediate in this payload names 0x801F3E64, so the absence of
 *           arguments is structural rather than declared.
 *
 *           Naming evidence: entry 1 of the same row, the exact
 *           incrementSpawnFourthExpiry,
 *           consumes the two halfwords this body arms - it post-increments the
 *           counter at +0x30 and tests the halfword at +0x2E on its low two bits
 *           before decrementing it - so +0x2E is the phase countdown in frames
 *           and +0x30 the phase frame counter. The exact reset callee's own
 *           prose records the same four steps of this span as the state it
 *           re-arms, and entry 1 reads the state byte at +0x01 that this span
 *           advances, which fixes the advance as the hand-off to the next entry.
 *
 *           Shape evidence: the 76-byte span is not byte-identical to any span
 *           of the corpus - a scan of every payload under out/binaries found the
 *           76-byte window equal only at offset 0x1264 of this payload, and no
 *           window of any payload matched it with this span's two jal immediates
 *           masked - so no clone applied and no near twin carried this body. The
 *           statement order of the arm block comes from this span's own bytes,
 *           where the state byte is loaded before the two halfword stores and
 *           written back after them; the cell reload that follows the call and
 *           the jr/nop tail follow the exact 3EB0 lift of this target, whose
 *           cell read, +0x2E and +0x30 halfword access, +0x01 advance and
 *           0x18-byte frame with $ra at 0x10 are the same.
 *
 *           Representation: the cell is spelled as this target's map name and
 *           read once, after the reset call, so its lui/lw pair follows that
 *           call's delay slot as the original's does; the state byte is kept in
 *           a u8 local and written back after the two stores, which is the order
 *           the original's lbu, addiu and sb carry; the two halfword stores are
 *           the literals 0x40 and 0; the cue argument is declared u16 as in the
 *           exact lifts that queue cue ids through 0x8015DF18. No alternative
 *           spelling was measured before the first gate: this body matched at
 *           rung 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePhaseFrameCounter(
    void) {
  u8* work;
  u8 state;

  stateOneShotLatches();
  work = scratchWorkPointerAt1;
  state = work[1];
  *(u16*)(work + 0x2E) = 0x40;
  *(u16*)(work + 0x30) = 0;
  work[1] = state + 1;
  func_8015DF18(0x200);
}
