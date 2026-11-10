#include "bof3/bof3.h"

/* @source 0x8015477C @kind function */
extern s16 func_8015477C(s32 x, s32 y);

typedef struct Area14313SignedPair {
  s8 x;
  s8 y;
} Area14313SignedPair;

/* @source 0x801F42E8 @kind data */
extern Area14313SignedPair D_801F42E8[];

typedef struct Area14313SeedRecord {
  u8 unk_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  s32 unk_04;
  s32 unk_08;
  s32 unk_0c;
  s32 unk_10;
} Area14313SeedRecord;

/* @source 0x801F2FE0
 * @behavior Initializes 16 records at 0x800E6000 with stride 0x14. Sets the
 *           lead byte to 1 and byte 1 to zero, seeds bytes 2 and 3 from rand
 *           OR 0xF0 and rand AND 0x1F, expands the signed coordinate pair at
 *           0x801F42E8 into words 4 and 8, and stores the shared helper result
 *           shifted left 16 into word 0x0C. Called during overlay setup.
 * @status partial
 * @match 79.17
 * @residual Frame is 0x20 rather than 0x30; body instructions otherwise match.
 */
void initialize16WorkRecordsFromSignedPairsWorld03Area14313_801F2FE0(void) {
  u8* flags;
  Area14313SeedRecord* record;
  u16 i;

  flags = PSX_PTR(u8, 0x800E6000u);
  i = 0;
  record = PSX_PTR(Area14313SeedRecord, 0x800E6000u);
  do {
    *flags = 1;
    record->unk_01 = 0;
    record->unk_02 = rand() | 0xF0;
    record->unk_03 = rand() & 0x1F;
    record->unk_04 = D_801F42E8[i].x << 16;
    record->unk_08 = D_801F42E8[i].y << 16;
    record->unk_0c = func_8015477C(record->unk_04, record->unk_08) << 16;
    flags += 0x14;
    i++;
    record++;
  } while (i < 0x10);
}
