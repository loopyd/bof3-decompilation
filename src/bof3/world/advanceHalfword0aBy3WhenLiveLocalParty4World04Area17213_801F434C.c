#include "bof3/bof3.h"

/* Shared live count of the 0x140-byte local work records based at 0x80145E90. */
extern u8 D_80146254; /* @source 0x80146254 @kind unknown */

/* Party byte of the 0x140-byte local work records of the run at 0x80145E90,
 * read here as the flat +0x79 word the payload's own lui/addiu pair names. */
extern u8 D_80145F09[]; /* @source 0x80145F09 @kind unknown */

extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* @source 0x801F434C
 * @behavior Walks the live 0x140-byte local work records of the run at
 *           0x80145E90 - the count byte D_80146254 bounds the walk and each
 *           record's party byte is the flat +0x79 word 0x80145F09 - and returns
 *           at the first record whose party byte reads 4, adding 3 to the
 *           halfword at 0x0A of the work record published through the global
 *           pointer cell 0x80148210 before it leaves. The published record is
 *           loaded once inside the count guard and the walk index is a 32-bit
 *           counter masked to a byte for both the scaled row offset and the
 *           loop bound. A walk that reaches the end without a party 4 leaves the
 *           halfword untouched. Takes no arguments, returns nothing and calls
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Seeded from the exact sibling
 * clearFlag40AndRequestResourceOnMode6Substate12ElseCopyStatus2WorkRecordTripletAndTickHalfword0aWorld02Area08513_801F37C0,
 * whose guarded do-while over this same record run emits the identical
 * preheader: the count byte in a local, the compared party literal held in a
 * local so its constant is materialised before the cell load, the published
 * record loaded once inside the guard, and the bound copied out of the count
 * load. This span's own bytes supply the party literal 4 and the +3.
 *
 * The found arm leaves through `return` rather than `break`: with `break` the
 * compiler inverted the walk into a top-tested loop (30 instructions, 38.71%,
 * first difference on the count load), while `return` keeps the guarded
 * do-while this payload shows and matched 31/31.
 */
void advanceHalfword0aBy3WhenLiveLocalParty4World04Area17213_801F434C(void) {
  u16* record;
  u8 count;
  u32 i;
  u32 offset;
  u32 state;

  i = 0;
  count = D_80146254;
  if (count != 0) {
    state = 4;
    record = D_80148210;
    do {
      offset = (u8)i * 0x140;
      if (D_80145F09[offset] == state) {
        record[5] += 3;
        return;
      }
      i++;
    } while ((u8)i < count);
  }
}
