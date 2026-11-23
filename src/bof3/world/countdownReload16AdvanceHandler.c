#include "bof3/bof3.h"

/* @source 0x801F4FB8
 * @behavior Frame countdown handler for one record of a work table: decrements
 *           the record byte at +0x02 as an unsigned byte (the store wraps at
 *           0xFF, and the `andi $v0,$v0,0xFF` after it is the truncation of the
 *           stored value), returns while the decremented count is nonzero, and
 *           when it reaches zero reloads that byte with 0x10 and increments the
 *           handler-index byte at +0x01, handing the record to the next handler
 *           of its row. The record pointer is the only argument and the
 *           function returns nothing; it is a leaf with no frame, and the
 *           constant is materialised in the branch delay slot of the countdown
 *           test.
 *
 *           Dispatch evidence: the only word in the payload naming this address
 *           is 0x801F5FC0 (`cmp` of the payload word against 0x801F4FB8), the
 *           second entry of the three-word handler row based at 0x801F5FBC
 *           (0x801F4F60, 0x801F4FB8, 0x801F4FEC; the row ends where the script
 *           blob at 0x801F5FC8 begins). func_801F4E7C walks the 16 records of
 *           stride 0x1C at 0x800E4800, skips the ones whose lead byte is zero,
 *           scales each record's byte +0x01 by four onto that 0x801F5FBC base,
 *           `jalr`s the selected entry with the record pointer in $a0, and then
 *           calls func_801F4BC0 with the same record. No `jal` immediate names
 *           this address, so the record pointer role comes from that indirect
 *           call site and stays structural.
 *
 *           Shape evidence: the 0x34-byte span is word-identical to the sibling
 *           span at 0x801F8730 in emi/scenario/scena07/00 except for the one
 *           reload constant (0x10 here, 0x20 there), which a payload `cmp` of
 *           the two spans showed as a single differing byte at offset 0x18;
 *           that sibling's own body was the statement order and the declaration
 *           style this seed follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownReload16AdvanceHandler(u8* record) {
  if (--record[2] == 0) {
    record[2] = 0x10;
    record[1]++;
  }
}
