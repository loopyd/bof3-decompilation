#include "bof3/bof3.h"

/* This overlay's trail rebuild at 0x801F2F28: it shifts the 0x20-record trail
 * buffer passed in a0, reprojects every record from the work object published at
 * the scratchpad pointer cell and fills each record's heading and size. */
void stateHeadingsAverageAngles(
    u8 *arg0);

/* This overlay's trail renderer, still on its assembled boundary, at 0x801F3258:
 * it walks the same record buffer and emits the run of fading translucent
 * gouraud fans and semi-transparent G4 quads through the PsyQ primitive
 * cursors. */
extern void renderTrailFan(u8 *arg0); /* @source 0x801F3258 @kind function */

/* Work-record pointer cell of the shared scratchpad, this target's map row for
 * 0x1F800044. It stays unqualified: every store in this body goes through a
 * pointer derived from this cell, so the compiler must re-read the cell for each
 * of the three accesses, and each access re-materialises the cell address. */
extern u8 *scratchpadWorkRecord; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F2D80
 * @behavior Entry 2 of this overlay's work-byte-1 handler table: the dispatcher
 * at 0x801F2C04 scales the work record's byte 0x01 by four and calls that entry
 * of the 0x801F58DC table, whose words at 0x801F58DC, 0x801F58E0, 0x801F58E4 and
 * 0x801F58E8 are 0x801F2C48, 0x801F2CE4, this address and 0x801F2E04. It first
 * rebuilds the trail buffer at the fixed main-RAM address
 * 0x800E4800 through this overlay's 0x801F2F28 walk - the buffer the four
 * in-image call sites of that address pass - and then hands the same buffer to
 * the overlay's renderer at 0x801F3258, which emits the fading translucent fan
 * and semi-transparent quad run. It then takes the work record published at the
 * scratchpad pointer cell 0x1F800044 and decrements its byte 0x09 modulo 256;
 * when the decremented byte reads zero it reloads that byte with 0x10 - the
 * 16-tick rearm - and advances the same record's byte 0x01 by one, so the next
 * frame dispatches entry 3 at 0x801F2E04. The cell is reloaded for the byte-1
 * store, matching the original's three separate cell accesses, and the record
 * pointer is kept in one register for the decrement and its store. Takes no
 * arguments and returns nothing; its 0x18-byte frame only holds $ra at 0x10
 * across the two calls, whose delay slots stay nop.
 *
 * Name evidence: no jal in the payload encodes this address - the only word
 * naming it is the table word at 0x801F58E4 - so it is a dispatch entry, not a
 * called helper, and its entry index 2 is the work-byte-1 value that runs it.
 * The two callees supply the verb pair: 0x801F2F28 is the exact
 * stateHeadingsAverageAngles,
 * whose own metadata records 0x800E4800 as the buffer all four in-image callers
 * pass, and 0x801F3258 is anchored by address because it has no reviewed body.
 * The tail's reload-and-advance pair follows the statement order,
 * `countdown = (u8)(work[9] - 1); work[9] = countdown;` tested for zero and the
 * state byte read again through the cell of the exact sibling
 * claimWorkRecordEveryFourthCountdownTickThenAdvanceWorkByte1World03Area13413_801F4288;
 * this body's own gate operand 0x10, its zero test and its reload points come
 * from the original bytes.
 *
 * Representation notes: the losing spelling was the scratchpad macro pair,
 * `work = SPAD_PTR_SLOT(u8, 0x44);` with two further `SPAD_PTR_SLOT(u8, 0x44)`
 * accesses, which measured 26/33 instructions with the cell address
 * 0x1F800044 materialised once into a register and reused by all three loads,
 * against the original's three separate address materialisations; that also
 * cost one instruction, 132 -> 128 bytes. Naming the cell and qualifying it
 * `u8 *volatile` produced the three separate `lui`/`lw` pairs but measured
 * 31/33: the 0x10 rearm was scheduled after the second cell load and left a
 * `nop` in the `bnez` delay slot, where the original fills that slot with the
 * rearm constant. The unqualified cell declaration matched every word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void rebuildDrawTrailAdvance(
    void) {
  u8 *work;
  u8 countdown;

  stateHeadingsAverageAngles(
      PSX_PTR(u8, 0x800E4800u));
  renderTrailFan(PSX_PTR(u8, 0x800E4800u));
  work = scratchpadWorkRecord;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    scratchpadWorkRecord[9] = 0x10;
    scratchpadWorkRecord[1]++;
  }
}
