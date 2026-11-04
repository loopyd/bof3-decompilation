#include "bof3/bof3.h"

/* Record table at 0x801F0A38 in this overlay's trailing read-only block: four
 * bytes per record, 32 records per 128-byte row. The row pitch is the sll 7 of
 * work[0x4] in this function. The unlifted site at 0x801F02A0 still reads the
 * same two halfwords from asm, so splat keeps binding this address through the
 * generated undefined_syms_auto.txt and the target map needs no row. */
typedef struct BmagicMagic04403PairRecord {
  u16 unk_00;
  u16 unk_02;
} BmagicMagic04403PairRecord;

extern BmagicMagic04403PairRecord D_801F0A38[][32]; /* @source 0x801F0A38 @kind data */

/* @source 0x801F0390
 * @behavior Record-slot insert of this overlay's pair table: it takes no
 *           arguments and returns nothing. The scratchpad pointer cell at
 *           0x1F800044 is read into a work pointer, and from that work object
 *           the function takes the one-past-last record index in byte 0xA, the
 *           lowest index in byte 0xB and the row selector in byte 0x4. It copies
 *           every record of that row from index work[0xB] through work[0xA] - 1
 *           one record higher (record i overwrites record i + 1, running
 *           downwards so the copy needs no overlap check), re-reading the row
 *           byte for the second halfword of each record because the first store
 *           may alias the work object. It then re-reads the pointer cell into a
 *           second pointer and stores the -1 sentinel into both halfwords of
 *           record work[0xB], which opens an empty slot at the low index and
 *           pushes the rest of the row up by one. The sibling site at 0x801F02A0
 *           has the same body with the same table and instead inserts the work
 *           object's halfword pair at 0x2E and 0x30; it is not part of this lift.
 *           Shape from the bytes: the table address is the record table plus the
 *           128-byte row offset (sll 7) and the 4-byte record offset (sll 2), and
 *           the destination record offset is the strength-reduced (i + 1) * 4
 *           that the original keeps in $a2 and decrements in the loop-latch
 *           branch delay slot. The loop is a for loop over an s32 index with the
 *           entry guard in the prologue (slt plus bnez, whose delay slot holds
 *           the first sll) and the repeated bottom test, which re-reads work[0xB]
 *           after the stores. The two pointer reads are separate locals: the
 *           original keeps the loop pointer in $a3 and the exit pointer in $a0,
 *           so one re-assigned variable would coalesce them into one register.
 *           The sentinel reaches the two u16 stores from a signed scalar, which
 *           is why the original materializes it with addiu $a1,$zero,-1 and not
 *           the ori of a direct 0xFFFF constant. The function is a leaf with no
 *           frame and no saved registers, ends in a plain jr $ra with a nop delay
 *           slot and leaves no result in $v0. Its only in-image caller is
 *           func_801EF478 (jal at 0x801EF480).
 * @status exact
 * @match 100.00
 * @residual none
 */
void insertEmptyRecordAtWorkByteBAndShiftRowRecordsUpBmagicMagic04403_801F0390(void) {
  u8 *work;
  u8 *slot;
  s32 i;
  s16 empty;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  for (i = work[0xA] - 1; i >= work[0xB]; i--) {
    D_801F0A38[work[4]][i + 1].unk_00 = D_801F0A38[work[4]][i].unk_00;
    D_801F0A38[work[4]][i + 1].unk_02 = D_801F0A38[work[4]][i].unk_02;
  }
  slot = SPAD_PTR_SLOT(u8, 0x44u);
  empty = -1;
  D_801F0A38[slot[4]][slot[0xB]].unk_00 = empty;
  D_801F0A38[slot[4]][slot[0xB]].unk_02 = empty;
}
