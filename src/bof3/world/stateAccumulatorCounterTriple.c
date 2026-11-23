#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. This body reads it once for the whole
 * accumulator update, so the two record words share that one load. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Pointer cell of the shared work record whose byte 0x13D selects the 12-byte
 * counter record this step advances. */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* Base of the 12-byte counter records selected by byte 0x13D of the shared
 * record: bytes 0x0, 0x1 and 0x2 are the three counters this step adds 2 to,
 * which the original reaches as this base, this base plus 1 and the generated
 * row at 0x80145BD8 (this base plus 2). */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */

/* @source 0x801F4118
 * @behavior Per-frame step of the work record published at the scratchpad
 * pointer cell 0x1F800044: it adds the record step word at 0x0C into its
 * accumulator word at 0x34 through one read of that cell (lw of 0x34, lw of
 * 0x0C, addu, sw), then adds 2 to each of the three counter bytes 0x0/0x1/0x2
 * of the 12-byte record that byte 0x13D of the record published through the
 * global pointer cell 0x80146250 selects in the 0x80145BD6 table. Each of the
 * three accesses re-reads 0x80146250 (its own lui/lw pair) and scales the
 * selected byte by the 12-byte stride (sll 1, addu, sll 2) into a byte offset,
 * then performs a byte-width read-modify-write (lbu, addiu $v0,$v0,2, sb) with
 * the table base re-materialised for the load and again for the store, which is
 * why the counter displacements fold into the base symbol's own relocation.
 * The record step word and the shared pointer are read into their own locals
 * because the original reads 0x80146250 ahead of the accumulator store and
 * carries that pointer in the register the store cannot use; writing the
 * update as one compound assignment left that read after the store and
 * measured 53 of 56 instructions before this form matched all 56. The body is a
 * leaf with no frame and no calls. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAccumulatorCounterTriple(
    void) {
  s32 accum;
  s32 step;
  u32 offset;
  u8* shared;
  u8* work;

  work = D_1F800044;
  accum = *(s32*)(work + 0x34);
  step = *(s32*)(work + 0x0c);
  shared = D_80146250;
  *(s32*)(work + 0x34) = accum + step;
  offset = shared[0x13d] * 12;
  D_80145BD6[offset] += 2;
  offset = D_80146250[0x13d] * 12;
  D_80145BD6[offset + 1] += 2;
  offset = D_80146250[0x13d] * 12;
  D_80145BD6[offset + 2] += 2;
}
