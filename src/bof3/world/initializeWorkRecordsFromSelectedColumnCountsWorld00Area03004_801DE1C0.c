#include "bof3/world/area03004_internal.h"

extern u8 D_80143F1C; /* @source 0x80143F1C @kind unknown */
extern u8 workRecordCountsBySelectorAndColumnWorld00Area03004[]; /* @source 0x801E2DF4 @kind table */

/*
 * @source 0x801DE1C0
 * @behavior Walks 23 count-table columns selected by D_80143F1C,
 * initializes consecutive work records, seeds their halfword 0x90
 * amounts from rand(), derives halfwords 0x8C/0x8E, optionally
 * increments the seeded amount, and clears work bytes 1-4 and 0x5C.
 * @status partial
 * @match 43.85
 * @residual Complete native bytes still differ. The original 748-byte
 * interval remains retained until exact reconstruction.
 */
void initializeWorkRecordsFromSelectedColumnCountsWorld00Area03004_801DE1C0(void) {
  s32 column;
  s32 recordIndex;
  s32 remaining;
  s32 attempts;
  s32 i;
  s32 scaled;
  u32 limit;
  u32 half;
  u32 slot;
  u16 amount;
  u8* record;
  u8* work;
  Area030Range* range;
  Area030ScaleRecord* row;

  recordIndex = 0;
  for (column = 0; column < 0x17; column++) {
    remaining = workRecordCountsBySelectorAndColumnWorld00Area03004[D_80143F1C * 0x17 + column];
    if (remaining > 0) {
      do {
        record = (u8*)&D_80146888[recordIndex];
        D_1F800044 = record;
        D_80146884 = (Area030Range*)record;
        record[0] = 0x21;

        work = D_1F800044;
        work[5] = (u8)recordIndex;
        work = D_1F800044;
        work[6] = (u8)column;

        amount = (u8)rand();
        record = (u8*)D_80146884;
        work = D_1F800044;
        for (;;) {
          *(u16*)(record + 0x90u) = amount;
          limit = D_801E2AB8[work[6]].limit;
          amount = *(u16*)(record + 0x90u);
          half = limit >> 1;

          if (limit < amount) {
            amount = (u16)(amount - half);
            continue;
          }
          if (amount < half) {
            amount = (u16)half;
          }
          *(u16*)(record + 0x90u) = amount;
          break;
        }

        range = D_80146884;
        work = D_1F800044;
        row = &D_801E2AB8[work[6]];
        amount = *(u16*)((u8*)range + 0x90u);
        scaled = (s32)row->unk_1C[2] *
                 (((s32)amount * 10) / (s32)row->limit);
        scaled /= 10;
        range->limit_8C = (s16)scaled;
        range->value_8E = (s16)scaled;

        slot = work[6];
        if (slot != 0x15u) {
          limit = D_801E2AB8[slot].limit;
          if (*(u16*)((u8*)range + 0x90u) == limit) {
            attempts = (u8)(limit / 5u);
            for (i = 0; i <= attempts; i++) {
              if ((rand() & 1) == 0) {
                break;
              }
              range = D_80146884;
              *(u16*)((u8*)range + 0x90u) += 1;
            }
          }
        }

        work = D_1F800044;
        work[1] = 0;
        work = D_1F800044;
        work[2] = 0;
        work = D_1F800044;
        work[3] = 0;
        work = D_1F800044;
        work[4] = 0;
        work = D_1F800044;
        work[0x5C] = 0;

        recordIndex++;
        remaining--;
      } while (remaining > 0);
    }
  }
}
