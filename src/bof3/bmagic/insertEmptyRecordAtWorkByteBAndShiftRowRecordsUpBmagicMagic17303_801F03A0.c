#include "bof3/bof3.h"

/* Record table at 0x801F09B0 in this overlay's trailing read-only block: four
 * bytes per record, 32 records per 128-byte row. The row pitch is the sll 7 of
 * work[0x4] in this function. The unlifted sites at 0x801EEEEC, 0x801EF4EC,
 * 0x801EFA34 and 0x801F02B0 still read the same two halfwords from asm, so
 * splat keeps binding this address through the generated
 * undefined_syms_auto.txt and the target map needs no row. */
typedef struct BmagicMagic17303PairRecord {
  u16 unk_00;
  u16 unk_02;
} BmagicMagic17303PairRecord;

extern BmagicMagic17303PairRecord D_801F09B0[][32]; /* @source 0x801F09B0 @kind data */

/* @source 0x801F03A0
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
 *           pushes the rest of the row up by one. Shape from the bytes: the
 *           table address is the record table plus the 128-byte row offset
 *           (sll 7) and the 4-byte record offset (sll 2), and the destination
 *           record offset is the strength-reduced (i + 1) * 4 that the original
 *           keeps in $a2 and decrements in the loop-latch branch delay slot. The
 *           loop is a for loop over an s32 index with the entry guard in the
 *           prologue (slt plus bnez, whose delay slot holds the first sll) and
 *           the repeated bottom test, which re-reads work[0xB] after the stores.
 *           The two pointer reads are separate locals: the original keeps the
 *           loop pointer in $a3 and the exit pointer in $a0, so one re-assigned
 *           variable would coalesce them into one register. The sentinel reaches
 *           the two u16 stores from a signed scalar, which is why the original
 *           materializes it with addiu $a1,$zero,-1 and not the ori of a direct
 *           0xFFFF constant. The function is a leaf with no frame and no saved
 *           registers, ends in a plain jr $ra with a nop delay slot and leaves no
 *           result in $v0. Its only in-image caller is func_801EF484 (jal at
 *           0x801EF48C), which passes no arguments and reads no result.
 *
 *           Body and statement order come from the exact cross-overlay sibling
 *           at 0x801F0390 in the magic044 #3 payload, whose 0xE8-byte span is
 *           this one with the table rebased: a byte cmp of the two 232-byte
 *           payload spans differs only in the six %lo displacements of the table
 *           base (0x801F09B0/0x801F09B2 here against 0x801F0A38/0x801F0A3A
 *           there), so that sibling is shape evidence rather than a clone, and
 *           this function's own table symbol and address are written from these
 *           bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void insertEmptyRecordAtWorkByteBAndShiftRowRecordsUpBmagicMagic17303_801F03A0(void) {
  u8 *work;
  u8 *slot;
  s32 i;
  s16 empty;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  for (i = work[0xA] - 1; i >= work[0xB]; i--) {
    D_801F09B0[work[4]][i + 1].unk_00 = D_801F09B0[work[4]][i].unk_00;
    D_801F09B0[work[4]][i + 1].unk_02 = D_801F09B0[work[4]][i].unk_02;
  }
  slot = SPAD_PTR_SLOT(u8, 0x44u);
  empty = -1;
  D_801F09B0[slot[4]][slot[0xB]].unk_00 = empty;
  D_801F09B0[slot[4]][slot[0xB]].unk_02 = empty;
}
