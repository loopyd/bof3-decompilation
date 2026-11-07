#include "bof3/bof3.h"

/* Opaque 0x98-byte work-record view. The shared game work-area header declares
 * the record array at 0x80146888 as an array of the 0x98-byte record, and the
 * original publishes the index of a record as a pointer difference between
 * records, which GCC 2.7.2 turns into that stride's exact division. The field
 * roles at the offsets this body touches are not established, so they stay
 * offset-anchored. */
typedef struct WorkRecordBytes {
  u8 bytes[0x98];
} WorkRecordBytes;

/* Pointer cell publishing the active work record; every record access below
 * re-reads it because a record store may alias the cell. */
extern WorkRecordBytes *D_80148208; /* @source 0x80148208 @kind data */
extern WorkRecordBytes D_80146888[]; /* @source 0x80146888 @kind data */
extern u8 D_80146867; /* @source 0x80146867 @kind data */
extern s8 D_801448EB; /* @source 0x801448EB @kind data */
extern s8 D_801448ED; /* @source 0x801448ED @kind data */

void func_8015C088(void);

/* @source 0x801F3F48
 * @behavior Overlay entry handler: runs the shared front-end startup helper at
 *           0x8015C088, then resets three fields of the work record published
 *           through the pointer cell 0x80148208 by storing 4 into its byte at
 *           +1, 5 into its byte at 0x77 and 0 into its halfword at 0x7E. It then
 *           publishes into the shared byte 0x80146867 the position of that
 *           record inside the 0x98-byte record array based at 0x80146888, and
 *           finally writes 4 into the shared front-end state byte 0x801448EB and
 *           3 into the shared state byte 0x801448ED, leaving the middle byte
 *           0x801448EC untouched. Takes no arguments and returns 0.
 * @status partial
 * @match 51.28
 * @residual Same 156 bytes, same 39-instruction multiset and the same memory
 *           events as the original; 19 instructions differ, all of them by
 *           register naming (the original keeps the three reloaded record
 *           pointers and the chain temporaries in v0/v1/a0/a1 and the shared 4
 *           in a2, this build folds them into v0/v1 plus a0) except the last
 *           two shared-state stores, which the original emits as 0x801448EB then
 *           0x801448ED and this build emits in the opposite order.
 *
 * The index store is the original's element difference between two 0x98-byte
 * record pointers, so the stride division appears as the multiply, negate and
 * arithmetic shift GCC 2.7.2 emits for that exact division; the same sequence
 * publishes a record index in emi/world00/area032/13 at 0x801F3C18 and in
 * emi/world00/area008/13 at 0x801F4424, both of which spell it as a pointer
 * difference of that record type. The nearest shape sibling is
 * emi/world02/area098/13 at 0x801F5EBC, the same 156 bytes with 0 into byte 0x77
 * and 9 into 0x801448ED, which is still unlifted (its own Splat row is `asm`), so
 * this body is seeded here rather than cloned.
 *
 * Residual class: allocation and scheduling, not representation. The record
 * pointer cell is re-read per access because a record store may alias it, the
 * pointer difference precedes the halfword clear in source, letting the compiler
 * fold their cell reads into one load and emit the clear before the subtraction,
 * and the pointer difference has to stay a pointer difference to keep the exact
 * division. The single-execution scope around the difference and clear keeps
 * those operations together. Under the owning GCC 2.7.2 profile its loop notes
 * change arithmetic reference weights and allocation without adding instructions.
 * The arithmetic shift remains signed; narrowing the index would lose it.
 */
s32 resetWorkRecordAndPublishRecordIndexThenState4Substate3World01Area06813_801F3F48(
    void) {
  s32 index;

  func_8015C088();
  ((u8 *)D_80148208)[1] = 4;
  ((u8 *)D_80148208)[0x77] = 5;
  do {
    index = D_80148208 - D_80146888;
    *(u16 *)((u8 *)D_80148208 + 0x7E) = 0;
  } while (0);
  D_80146867 = (u8)index;
  D_801448EB = 4;
  D_801448ED = 3;
  return 0;
}
