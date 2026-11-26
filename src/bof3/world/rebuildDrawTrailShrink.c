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

/* Size seed of the trail buffer, the halfword entry 0x1F of the size run the
 * 0x801F2F28 walk copies into every record: entry 0 of the work-byte-1 handler
 * table at 0x801F2C48 seeds it with 0x60 before its rebuild loop, and this span
 * is the only other access in the payload. It sits in the fixed main-RAM trail
 * area opened at 0x800E4800, so it stays address-anchored. */
extern u16 D_800E4B3E; /* @source 0x800E4B3E @kind data */

/* Work-record pointer cell of the shared scratchpad, this target's map row for
 * 0x1F800044. It stays unqualified: the body derives two pointers from this
 * cell, so the compiler must re-read the cell for each access and each access
 * re-materialises the cell address. */
extern u8 *scratchpadWorkRecord; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F2E04
 * @behavior Entry 3 of this overlay's work-byte-1 handler table: the dispatcher
 * at 0x801F2C04 scales the work record's byte 0x01 by four and calls that entry
 * of the 0x801F58DC table, whose four words are 0x801F2C48, 0x801F2CE4,
 * 0x801F2D80 and this address. Like entry 2 it first rebuilds the trail buffer
 * at the fixed main-RAM address 0x800E4800 through this overlay's 0x801F2F28
 * walk - the buffer the in-image call sites of that address pass - and then
 * hands the same buffer to the overlay's renderer at 0x801F3258, which emits the
 * fading translucent fan and semi-transparent quad run. It then shrinks the
 * trail's size seed: the halfword at 0x800E4B3E, which entry 0 at 0x801F2C48
 * seeds with 0x60 before its 0x20-iteration rebuild loop and which the 0x801F2F28
 * walk copies into every record's size, loses 6 here, so successive frames
 * narrow the trail. It then takes the work record published at the scratchpad
 * pointer cell 0x1F800044 and decrements its byte 0x09 modulo 256; when the
 * decremented byte reads zero it advances the same record's byte 0x01 by one,
 * so the next frame dispatches entry 4 at 0x801F2E88. Unlike entry 2 this span
 * writes no 0x10 rearm, so the expired state is left at zero. The cell is
 * reloaded for the byte-1 advance, and the record pointer is kept in one
 * register for the decrement and its store. Takes no arguments and returns
 * nothing; its 0x18-byte frame only holds $ra at 0x10 across the two calls,
 * whose delay slots carry the 0x800E4800 argument setup.
 *
 * Name evidence: no jal in the payload encodes this address - the only word
 * naming it is the table word at 0x801F58E8 - so it is a dispatch entry, not a
 * called helper, and its entry index 3 is the work-byte-1 value that runs it.
 * The two callees supply the verb pair, the same pair entry 2 at 0x801F2D80
 * uses: 0x801F2F28 is the exact
 * stateHeadingsAverageAngles,
 * which records 0x800E4800 as the buffer all four in-image callers pass, and
 * 0x801F3258 is anchored by address because it has no reviewed body. The shrunk
 * halfword is that walk's size seed rather than a trail record field: entry 0
 * stores 0x60 to it immediately before its rebuild loop, and the walk's own
 * metadata records it as the size it copies into each record. The three-way
 * difference against entry 2 - this size-seed block and the missing 0x10 rearm -
 * carries the name: this span rebuilds and draws the trail, shrinks the seed and
 * only advances the work byte when the countdown expires.
 *
 * Shape evidence: entry 2 at 0x801F2D80 is an exact lift of the same statement
 * order - the two 0x800E4800 calls, the work cell read, the byte-0x09 countdown
 * accumulator with its 0xff mask and zero test, and the byte-0x01 advance - so
 * that body supplied this one's order, operand forms and declaration style; this
 * span's own literals (the -6 decrement, the 0x800E4B3E operand and the dropped
 * 0x10 store) come from these bytes. No byte-identical sibling exists: the
 * 0x800E4B3E operand pair occurs only in this payload across out/binaries, and
 * the same-address span at 0x801F2E04 of emi/world00/area026/13 carries a
 * different body, so the clone lever is ineligible here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void rebuildDrawTrailShrink(
    void) {
  u8 *work;
  u8 countdown;
  u16 seed;

  stateHeadingsAverageAngles(
      PSX_PTR(u8, 0x800E4800u));
  renderTrailFan(PSX_PTR(u8, 0x800E4800u));
  seed = D_800E4B3E;
  work = scratchpadWorkRecord;
  seed = (u16)(seed - 6);
  D_800E4B3E = seed;
  countdown = (u8)(work[9] - 1);
  work[9] = countdown;
  if (countdown == 0) {
    scratchpadWorkRecord[1]++;
  }
}
