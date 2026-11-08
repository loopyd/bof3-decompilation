#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared live count of the 0x140-byte local work records based at 0x80145E90. */
extern u8 D_80146254; /* @source 0x80146254 @kind unknown */

/* 0x140-byte local work records; only the party byte this body reads is
 * proven here. */
typedef struct Area17213LocalRecord {
  u8 unk_00[0x79];
  u8 party_79;
  u8 unk_7a[0xc6];
} Area17213LocalRecord;

extern Area17213LocalRecord D_80145E90[]; /* @source 0x80145E90 @kind table */

/* Six 0x98-byte game work records. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind bss */

void func_8014D290(void);

/* @source 0x801F2ED8
 * @behavior Publishes every work record in turn through the scratchpad pointer
 *           cell 0x1F800044, writes the ordering-table index 2 into byte 0x29
 *           of the published record and then runs the shared main-executable
 *           service func_8014D290 with that record still published. The first
 *           pass covers the six 0x98-byte game work records based at
 *           0x80146888; the second pass runs only while the shared live count
 *           D_80146254 reads non-zero and covers the live 0x140-byte local work
 *           records based at 0x80145E90 whose party byte at +0x79 reads 7,
 *           skipping the records whose party byte differs. The cell value read
 *           on entry is restored last, so the caller's current record survives,
 *           and the count byte is re-read for every loop test. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte 0x29 is the ordering-table index the shared append helper 0x8014E5A0
 * takes as its ot_index for this record, so both passes route the records they
 * touch into bucket 2. Retained shapes: the second pass carries its own
 * pointer local instead of sharing the first pass pointer, and it reads the
 * party byte as the struct member at +0x79 rather than through a second
 * symbol, because either substitute moved the record base into the product
 * register and left three instructions different.
 */
void publishWorkRecordsWithOtIndex2AndRunSharedHandlerWorld04Area17213_801F2ED8(
    void) {
  u8* saved;
  u8* game_record;
  u8* local_record;
  u32 index;
  u32 i;

  saved = D_1F800044;

  for (i = 0; (i & 0xFF) < 6; i++) {
    game_record = &D_80146888[(i & 0xFF) * 0x98];
    D_1F800044 = game_record;
    game_record[0x29] = 2;
    func_8014D290();
  }

  i = 0;
  if (D_80146254 != 0) {
    do {
      index = i & 0xFF;
      if (D_80145E90[index].party_79 == 7) {
        local_record = (u8 *)&D_80145E90[index];
        D_1F800044 = local_record;
        local_record[0x29] = 2;
        func_8014D290();
      }
      i++;
    } while ((u32)(i & 0xFF) < D_80146254);
  }

  D_1F800044 = saved;
}
