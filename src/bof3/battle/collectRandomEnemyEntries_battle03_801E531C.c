#include "bof3/battle/battle03_internal.h"

extern int rand(void);
extern u8 D_80146327; /* @source 0x80146327 @kind unknown */
extern u16 D_80146334[]; /* @source 0x80146334 @kind unknown */
extern u8 D_80146354[]; /* @source 0x80146354 @kind unknown */
typedef struct Battle03EntryThresholds {
  u8 bytes[8];
} Battle03EntryThresholds;
typedef struct Battle03RandomEntry {
  u16 unk_00;
  u8 unk_02;
  u8 unk_03;
} Battle03RandomEntry;
typedef struct Battle03RandomEntryView {
  u8 pad_00[0x98];
  Battle03RandomEntry entries[2];
} Battle03RandomEntryView;
extern const Battle03EntryThresholds D_801D0CB8; /* @source 0x801D0CB8 @kind data */

/* @source 0x801E531C
 * @behavior Samples two current-enemy entries at +0x98/+0x9A against byte
 * thresholds, merges accepted halfword IDs into the counted list, and clears
 * each accepted source ID. The list length saturates at 15.
 * @status exact
 * @match 100.00
 * @residual none
 */
void collectRandomEnemyEntries_battle03_801E531C(void) {
  Battle03EntryThresholds thresholds;
  u8 slot;
  u8 entry;
  u8* count;
  u16* ids;
  s32 rate;

  thresholds = D_801D0CB8;
  slot = 0;
  ids = D_80146334;
  count = (u8*)ids - 13;
  do {
    if (((Battle03RandomEntryView*)battleCurrentEnemyWorkState)->entries[slot].unk_00 != 0) {
      rate = ((Battle03RandomEntryView*)battleCurrentEnemyWorkState)->entries[slot].unk_02;
      if (rate == 0 || thresholds.bytes[rate] < (rand() & 0xff)) {
        goto next_entry;
      }
      entry = 0;
      if (D_80146327 != 0) {
        do {
          if (ids[entry] ==
              ((Battle03RandomEntryView*)battleCurrentEnemyWorkState)->entries[slot].unk_00) {
            D_80146354[entry] += 1;
            ((Battle03RandomEntryView*)battleCurrentEnemyWorkState)->entries[slot].unk_00 = 0;
            goto next_entry;
          }
          entry += 1;
        } while (entry < *((u8*)ids - 13));
      }
      ids[*count] =
          ((Battle03RandomEntryView*)battleCurrentEnemyWorkState)->entries[slot].unk_00;
      D_80146354[*count] = 1;
      if (*count < 15) {
        *count += 1;
      }
      ((Battle03RandomEntryView*)battleCurrentEnemyWorkState)->entries[slot].unk_00 = 0;
    }
  next_entry:
    slot += 1;
  } while (slot < 2);
}
