#include "bof3/bof3.h"

typedef struct Area14313SpawnRecord {
  u8 unk_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  s32 unk_04;
  s32 unk_08;
  s32 unk_0c;
  s32 unk_10;
  s32 unk_14;
} Area14313SpawnRecord;

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

/* @source 0x801F2D7C @kind function */
extern Area14313SpawnRecord* func_801F2D7C(void);

/* @source 0x801F30A0
 * @behavior Scans 16 seed records at 0x800E6000 with stride 0x14. For
 *           each active seed, decrements byte 3 and, when its old value is
 *           zero, attempts to allocate two records from the 0x800E4800 pool.
 *           Seeds their flags, lifetime, randomized coordinates and word
 *           0x0C, clears word 0x14, and reloads the seed countdown from rand.
 *           Returns whether any seed was active.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 spawnWorkRecordPairsFromActiveSeedCountdownsWorld03Area14313_801F30A0(void) {
  s32 active;
  u8* flags;
  u16 i;
  Area14313SeedRecord* seed;
  u16 j;
  Area14313SpawnRecord* record;

  active = 0;
  flags = PSX_PTR(u8, 0x800E6000u);
  i = 0;
  seed = PSX_PTR(Area14313SeedRecord, 0x800E6000u);
  do {
    if (*flags != 0) {
      active = 1;
      if ((seed->unk_03)-- == 0) {
        j = 0;
        do {
          record = func_801F2D7C();
          if (record != 0) {
            record->unk_00 = 1;
            record->unk_02 = (rand() & 0x1F) + 0x20;
            record->unk_04 = seed->unk_04 +
                             ((rand() & 0xFF) << 8) - 0x10000;
            record->unk_08 = seed->unk_08 +
                             ((rand() & 0xFF) << 8) - 0x10000;
            record->unk_0c = seed->unk_0c;
            record->unk_14 = 0;
            record->unk_03 = rand() & 1;
          }
          j++;
        } while (j < 2);
        seed->unk_03 = rand() & 7;
      }
    }
    seed++;
    i++;
    flags += 0x14;
  } while (i < 0x10);
  return active;
}
