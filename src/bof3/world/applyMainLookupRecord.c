#include "bof3/core/slus_internal.h"

/* One published "main lookup" record. The leading halfword is the entry key the
 * selector matches; the remaining bytes are the fields copied into the active
 * work record. */
typedef struct MainLookupRecord {
  u16 id;
  u16 unk_02;
  u8  unk_04;
  u8  unk_05;
  u8  unk_06;
  u8  unk_07;
} MainLookupRecord;

/* Work-record fields written by the selector; the rest of the work area is
 * untouched here. */
typedef struct GameWorkRecord {
  u8  pad_00[0x25];
  u8  unk_25;
  u8  unk_26;
  u8  unk_27;
  u8  unk_28;
  u8  pad_29[0x02];
  u8  unk_2B;
  u16 unk_2C;
  u8  pad_2E[0x42];
  u32 unk_70;
} GameWorkRecord;

/* Published main lookup table base, filled in by the boot initializer. */
/* @source 0x801459F4 @kind data */
extern MainLookupRecord* D_801459F4;

/* Entry count for the table above. The original reaches it through a constant
 * address that it keeps in one register across the scan, so the literal stays
 * in this accessor. */
#define MAIN_LOOKUP_RECORD_COUNT PSX_REF(u8, 0x800E3800u)

/* Per-entry bytes copied into work word 0x70 and work byte 0x2B. Both tables are
 * addressed with the doubled entry id, so the two labels stay separate. */
/* @source 0x80181484 @kind data */
extern u8 D_80181484[];
/* @source 0x80181485 @kind data */
extern u8 D_80181485[];

/* @behavior Scans the published main lookup table for the record whose 16-bit
 * entry id matches. On a match it copies that record into the active work
 * record: bytes 0x25-0x28 and the halfword at 0x2C take the record's fields,
 * word 0x70 has its low byte cleared before the per-entry byte from D_80181484
 * replaces it, and byte 0x2B takes the byte from D_80181485. Returns 0 when the
 * record was found and 1 when the scan reaches the entry count without a match.
 * @source 0x8014DD3C
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 applyMainLookupRecord(u16 id) {
  s32 index;
  u16 matched;
  u32 row;

  for (index = 0; index < MAIN_LOOKUP_RECORD_COUNT; index++) {
    matched = D_801459F4[index].id;
    if (matched == id) {
      ((GameWorkRecord*)g_game_work)->unk_25 = D_801459F4[index].unk_04;
      ((GameWorkRecord*)g_game_work)->unk_26 = D_801459F4[index].unk_05;
      ((GameWorkRecord*)g_game_work)->unk_27 = D_801459F4[index].unk_02;
      ((GameWorkRecord*)g_game_work)->unk_28 = D_801459F4[index].unk_06;
      row = (u32)matched * 2;
      ((GameWorkRecord*)g_game_work)->unk_2C = D_801459F4[index].unk_07;
      ((GameWorkRecord*)g_game_work)->unk_70 &= 0xFFFFFF00u;
      ((GameWorkRecord*)g_game_work)->unk_70 = D_80181484[row];
      ((GameWorkRecord*)g_game_work)->unk_2B = D_80181485[row];
      return 0;
    }
  }
  return 1;
}
