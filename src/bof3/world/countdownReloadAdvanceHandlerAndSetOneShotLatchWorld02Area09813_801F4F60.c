#include "bof3/bof3.h"

extern u8 D_801F6510;

/* @source 0x801F4F60
 * @behavior Frame handler, entry 0 of the three-word handler row at 0x801F5FBC:
 *           adds 0x10 to the record byte at +0x03, then decrements the countdown
 *           byte at +0x02 as an unsigned byte (the store wraps and the mask after
 *           it is the truncation of the stored value). While that countdown stays
 *           nonzero it only returns; when it reaches zero it reloads the countdown
 *           from the record byte at +0x04, increments the handler-index byte at
 *           +0x01, handing the record to the next handler of its row, and sets the
 *           one-shot latch byte at 0x801F6510 to 1 if that latch still reads zero.
 *           The record pointer is the only argument and the function returns
 *           nothing; it is a leaf with no frame.
 *
 *           Dispatch evidence: the word 0x801F4F60 (cmp of the payload word
 *           against 0x801F4F60) is the first entry of the three-word handler row
 *           based at 0x801F5FBC (0x801F4F60, 0x801F4FB8, 0x801F4FEC; the row ends
 *           where the script blob at 0x801F5FC8 begins). func_801F4E7C walks the
 *           16 records of stride 0x1C at 0x800E4800, skips the ones whose lead
 *           byte is zero, scales each record's byte +0x01 by four onto that base,
 *           jalrs the selected entry with the record pointer in $a0, and then
 *           calls func_801F4BC0 with the same record. No jal immediate names this
 *           address, so the record pointer role comes from that indirect call site
 *           and stays structural.
 *
 *           Latch evidence: the exact reset handler
 *           resetWorkTableAt800E4800AndRearmOneShotLatchesWorld02Area09813_801F4D44
 *           zeroes 0x801F6510 and 0x801F6514 and reads both as one-shot flags that
 *           this address and func_801F4FEC set to 1 on their first pass, which is
 *           the naming evidence for the latch role here.
 *
 *           Shape evidence: the countdown half follows the statement order and
 *           operand forms of the exact
 *           countdownReload16ThenAdvanceHandlerWorld02Area09813_801F4FB8 lift in
 *           this target (same record, decrement, mask and handler advance), which
 *           reloads the constant 0x10 where this span reloads the record byte at
 *           +0x04, and this span additionally carries the +0x03 step and the latch
 *           store. The 0x58-byte span is not byte-identical to any span of the
 *           payload corpus (a scan of every payload under out/binaries for its
 *           first six words found only this copy), so no clone applied.
 *
 *           Representation note: the latch takes three spellings in the record
 *           above. The bare byte object D_801F6510 keeps the original per-access
 *           lui/%lo pair and measured 22/22 instructions. Casting the numeric
 *           address (PSX_REF of 0x801F6510u, the spelling the reset seed uses for
 *           its two stores) measured 18/22 with a shared lui/ori base in $a1, and
 *           indexing the same object declared as an unknown-size array measured
 *           18/22 with a lui/addiu base in $v1, because both materialise the whole
 *           address in a register once and reuse it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownReloadAdvanceHandlerAndSetOneShotLatchWorld02Area09813_801F4F60(
    u8* record) {
  record[3] += 0x10;
  if (--record[2] == 0) {
    record[2] = record[4];
    record[1]++;
    if (D_801F6510 == 0) {
      D_801F6510 = 1;
    }
  }
}
