#include "bof3/bof3.h"

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x1F800044 @kind data */
extern u8* volatile D_1F800044;

/* @source 0x801F3030
 * @behavior Seeding step for the scratch work record published at pointer cell
 *           0x1F800044: stores the word 0x4E8000 at +0x34, clears the word at
 *           +0x3C, then stores 0x88000 at +0x38 when the +0x79 byte of the
 *           current work record published through 0x80146250 equals 2 and
 *           0x80000 otherwise. It finishes by reloading the 0x1F800044 cell and
 *           storing 1 into the record's byte +0x08. Takes no arguments and
 *           returns nothing.
 *
 * Name evidence: the 0x34/0x38/0x3C word triple is the position triple that the
 * exact siblings of this overlay family seed and republish (for example
 * initializeWorkFieldsWorld02Area08213_801F4A18 converts a coordinate pair into
 * +0x34/+0x38 and clears +0x3C, and
 * claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0 records the
 * +0x34/+0x38 pair as the scratch work position). Byte +0x08 is the facing byte
 * that the exact sibling
 * alignWorkFacingByte8WithModeByteThenPublishPositionWorld03Area12114_801F5E2C
 * reads on the same published record, and the +0x79 byte of the record at
 * 0x80146250 carries the WorkByte79 spelling that the exact world02 siblings
 * advanceHalfword0aByWorkByte79World02Area08213_801F5594 and
 * advanceHalfword0aBy17WhenWorkByte79NonzeroWorld02Area10313_801F5738 use for
 * the same cell. This overlay's neighbour func_801F307C tests the same byte
 * against the same constant. No in-image caller exists: the overlay's own
 * dispatch tables reach their entries, so an unindexed setter of this shape is
 * ordinary for this payload.
 *
 * Shape evidence: no byte-identical sibling exists anywhere in the extracted
 * payload set (one search over out/binaries finds the span only in this target),
 * so the body is seeded from shape siblings. The decisive shape is the store in
 * both arms of the test: GCC cross-jumps the two identical stores into the one
 * store after the join that the original shows, and the delay-slot filler then
 * takes the else arm's `lui` while CSE drops the then arm's now redundant high
 * half. Binding the selection to a result local instead leaves the value in
 * $a1 and keeps four registers live.
 *
 * Cell spelling: the original re-materialises the 0x1F800044 cell address and
 * reloads the pointer after the two work stores, with no call in between, so the
 * cell is declared through the target-local volatile symbol that the exact
 * sibling claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0 uses for
 * the same reload, which keeps the `lui` + displacement `lw` encoding of both
 * loads.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkPositionSelectingYByWorkByte79ThenSetFacingByte8World03Area12813_801F3030(
    void) {
  u8* record;
  u8* work;

  work = D_1F800044;
  record = D_80146250;
  *(u32*)(work + 0x34) = 0x4E8000;
  *(u32*)(work + 0x3C) = 0;
  if (record[0x79] == 2) {
    *(u32*)(work + 0x38) = 0x88000;
  } else {
    *(u32*)(work + 0x38) = 0x80000;
  }
  D_1F800044[8] = 1;
}
