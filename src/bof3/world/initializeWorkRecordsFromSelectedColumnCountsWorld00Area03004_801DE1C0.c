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
 * @match 88.77
 * @residual The original pointer copy and signed-division trap expansion
 * still differ. The original 748-byte interval remains retained.
 */
void initializeWorkRecordsFromSelectedColumnCountsWorld00Area03004_801DE1C0(void) {
  s32 column;
  s32 recordIndex;
  s32 remaining;
  s32 attempts;
  s32 retryCount;
  s32 i;
  s32 scaled;
  u32 amountLimit;
  u32 half;
  u32 scaleOffset;
  u32 slot;
  u16 amount;
  s32 pendingAmount;
  s32 sampledAmount;
  u8* record;
  u8* amountRecord;
  u8* seedWork;
  u8* amountWork;
  u8* scaleWork;
  u8* clearWork;
  Area030Range* range;
  Area030Range* retryRange;

  column = 0;
  recordIndex = 0;
  for (; column < 0x17; column++) {
    amountWork = workRecordCountsBySelectorAndColumnWorld00Area03004;
    pendingAmount = D_80143F1C;
    amountLimit = pendingAmount << 1;
    amountLimit += pendingAmount;
    amountLimit <<= 3;
    amountLimit -= pendingAmount;
    amountLimit = (u32)(amountWork + amountLimit);
    amountLimit += column;
    remaining = *(u8*)amountLimit;
    if (remaining > 0) {
      do {
        record = (u8*)&D_80146888[recordIndex];
        D_1F800044 = record;
        D_80146884 = (Area030Range*)record;
        record[0] = 0x21;

        seedWork = D_1F800044;
        seedWork[5] = (u8)recordIndex;
        seedWork = D_1F800044;
        seedWork[6] = (u8)column;

        pendingAmount = rand() & 0xFF;
        amountRecord = (u8*)D_80146884;
        amountWork = D_1F800044;
        for (;;) {
          *(u16*)(amountRecord + 0x90u) = pendingAmount;
          pendingAmount = amountWork[6];
          amountLimit = pendingAmount << 3;
          amountLimit += pendingAmount;
          amountLimit <<= 2;
          amountLimit = *((u8*)D_801E2AB8 + amountLimit + 0x1Fu);
          sampledAmount = *(u16*)(amountRecord + 0x90u);
          if (amountLimit < (u16)sampledAmount) {
            pendingAmount = amountLimit >> 1;
            pendingAmount = sampledAmount - pendingAmount;
            continue;
          }
          half = amountLimit >> 1;
          amountLimit = sampledAmount;
          if ((u16)sampledAmount < half) {
            amountLimit = half;
          }
          *(u16*)(amountRecord + 0x90u) = amountLimit;
          break;
        }

        range = D_80146884;
        scaleWork = D_1F800044;
        scaleOffset = scaleWork[6] * sizeof(Area030ScaleRecord);
        amount = *(u16*)((u8*)range + 0x90u);
        scaled = (s32)*((u8*)D_801E2AB8 + scaleOffset + 0x1Eu) *
                 (((s32)amount * 10) /
                  (s32)*((u8*)D_801E2AB8 + scaleOffset + 0x1Fu));
        scaled /= 10;
        range->limit_8C = (s16)scaled;
        range->value_8E = (s16)scaled;

        slot = scaleWork[6];
        if (slot != 0x15u) {
          amountLimit = D_801E2AB8[slot].limit;
          if (*(u16*)((u8*)range + 0x90u) == amountLimit) {
            retryCount = (u8)(amountLimit / 10u);
            i = 0;
            if (i <= retryCount) {
              attempts = retryCount;
              do {
                if ((rand() & 1) == 0) {
                  break;
                }
                retryRange = D_80146884;
                *(u16*)((u8*)retryRange + 0x90u) += 1;
                i++;
              } while (i <= attempts);
            }
          }
        }

        clearWork = D_1F800044;
        clearWork[1] = 0;
        clearWork = D_1F800044;
        clearWork[2] = 0;
        clearWork = D_1F800044;
        clearWork[3] = 0;
        clearWork = D_1F800044;
        clearWork[4] = 0;
        clearWork = D_1F800044;
        clearWork[0x5C] = 0;

        recordIndex++;
        remaining--;
      } while (remaining > 0);
    }
  }
}
