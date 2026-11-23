#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell this overlay publishes. The original
 * materializes the cell with `lui` plus an offset load at each read and saves
 * no register for it across the cue call, so the body spells the cell itself
 * at both read points. */
extern u8* g_world02Area11113Work; /* @source 0x1F800044 @kind data */

/* Shared cue dispatcher of the main executable. This payload only calls it, so
 * the role comes from this call site. */
void func_8015DF18(u16 cue);

/* @source 0x801F2CA8
 * @behavior Entry handler 0 of the byte-1 dispatch table 0x801F4890 for the
 *           work record published at the scratchpad pointer cell 0x1F800044.
 *           It arms the record for the parts animation its successor handler
 *           steps: 0x28 into the countdown byte at offset 0x09, then zero into
 *           the signed halfword at offset 0x2E and 0x100 into the signed
 *           halfword at offset 0x30, all through one read of the pointer cell.
 *           It re-reads that cell after the countdown store, increments the
 *           record's dispatch byte at offset 0x01 and queues cue 0x206 through
 *           the shared cue dispatcher at 0x8015DF18, so the dispatcher at
 *           0x801F2C64 hands the next tick to handler 1. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra at 0x10 across
 *           the call.
 *
 * Name evidence: the handler index comes from the in-payload pointer at
 * 0x801F4890, slot 0 of the byte-1 table whose entries 0..3 are 0x801F2CA8,
 * 0x801F2CFC, 0x801F2D80 and 0x801F3130; the parts-animation role from the
 * halfword pair at 0x2E and 0x30 that the lifted handler 1 at 0x801F2CFC draws
 * and steps and from the countdown byte 0x09 that same body decrements; the
 * armed literals 0x28, 0 and 0x100 from the original `addiu`/`sh` immediates;
 * the dispatch byte 0x01 from the `lbu`/`addiu 1`/`sb` triple; and the cue from
 * the `addiu $a0, $zero, 0x206` in the delay slot of the call.
 *
 * Shape: written from the original bytes with the neighbouring handler
 * 0x801F2CFC as its statement-shape seed. A payload scan of every file under
 * out/binaries for this 84-byte span returned only this payload's own copy at
 * 0xA8, so no byte-identical sibling exists and the clone and carve levers do
 * not apply; the range carries its own prologue and `jr $ra`. The original
 * reads the cell twice - `lw $v1,0x44` for the three arming stores and
 * `lw $a0,0x44` between the countdown store and the halves store pair, feeding
 * only the dispatch-byte increment - so the body takes one local per read and
 * points the second read at its own statement.
 *
 * Representation notes: the first gate measured 16 of 21 instructions for the
 * one-read spelling, where a single pointer local served the arming stores and
 * the dispatch-byte increment re-read the cell from a later statement; the
 * second load stayed behind the halves stores and reused $v1, so the first
 * difference was the original's `lui $a0,0x1F80` at instruction 6. Declaring
 * that cell volatile measured the same 16 of 21. Hoisting the second read into
 * its own local ahead of the halves store group held both pointers live and
 * matched 21 of 21 instructions and 84 bytes with either the named cell or the
 * `SPAD_PTR_SLOT` accessor at that read; the named form stands because it is
 * this target's own binding for 0x1F800044 and the seeded sibling's spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePartsHalvesArm(
    void) {
  u8* work;
  u8* state;

  work = g_world02Area11113Work;
  work[9] = 0x28;
  state = g_world02Area11113Work;
  *(u16*)(work + 0x2E) = 0;
  *(u16*)(work + 0x30) = 0x100;
  state[1] += 1;
  func_8015DF18(0x206);
}
