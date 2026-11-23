#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. This body reads it once for the
 * accumulator update and twice for the countdown step. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Pointer cell of the shared work record whose byte 0x13D selects the 12-byte
 * counter record this step subtracts 2 from, and whose halfword 0x122 the
 * countdown arm consumes. */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */

/* Base of the 12-byte counter records selected by byte 0x13D of the shared
 * record: bytes 0x0, 0x1 and 0x2 are the three counters this step subtracts 2
 * from, which the original reaches as this base, this base plus 1 and the
 * generated row at 0x80145BD8 (this base plus 2). */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */

/* @source 0x801F3FDC
 * @behavior Per-frame step of the work record published at the scratchpad
 * pointer cell 0x1F800044: it adds the record step word at 0x0C into its
 * accumulator word at 0x34 through one read of that cell (lw of 0x34, lw of
 * 0x0C, addu, sw) with the global pointer cell 0x80146250 read ahead of the
 * store, then subtracts 2 from each of the three counter bytes 0x0/0x1/0x2 of
 * the 12-byte record that byte 0x13D of the record published through 0x80146250
 * selects in the 0x80145BD6 table. Each of those three accesses scales the
 * selected byte by the 12-byte stride (sll 1, addu, sll 2) into a byte offset
 * and performs a byte-width read-modify-write (lbu, addiu $v0,$v0,-0x2, sb)
 * with the table base re-materialised for the load and again for the store,
 * while the first access reuses the 0x80146250 pointer already live ahead of
 * the accumulator store and the second and third re-read it. It then decrements
 * the countdown byte at 0x0A of the work record; when that byte reads zero it
 * clears the record state byte at 0x04 to handler index 0 and returns, and
 * otherwise it consumes two units of the halfword at 0x122 of the record
 * published through 0x80146250. The state byte it clears is the index the
 * overlay's state-byte-4 dispatcher tests against the handler table based at
 * 0x801F567C, where this body is entry 3. Leaf with no frame and no calls;
 * takes no arguments and returns nothing. Reaching the countdown byte through
 * an assigned record local instead of these direct cell reads measured 65 of
 * 79 instructions, and a second distinct record local 75 of 79, while these
 * direct reads leave the head block's pointer in $v1 and match all 79.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAccumulatorTripleConsume(
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
  D_80145BD6[offset] -= 2;
  offset = D_80146250[0x13d] * 12;
  D_80145BD6[offset + 1] -= 2;
  offset = D_80146250[0x13d] * 12;
  D_80145BD6[offset + 2] -= 2;
  D_1F800044[0x0a] = D_1F800044[0x0a] - 1;
  if (D_1F800044[0x0a] == 0) {
    D_1F800044[4] = 0;
  } else {
    *(u16*)(D_80146250 + 0x122) -= 2;
  }
}
