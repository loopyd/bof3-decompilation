#include "bof3/ui/commu00_internal.h"
#include <stdlib.h>

/* One six-byte band entry of the container-local slot band table at
 * 0x801F2530: the halfword bound the band scan tests, the halfword gate the
 * resolved record's required elapsed battle count must reach, and the halfword
 * success weight the outcome draw scales by the record's stride-9 byte. */
typedef struct SlotOutcomeBand {
  u16 threshold;
  u16 requirement;
  u16 weight;
} SlotOutcomeBand;

/* @source 0x801F2530 @kind table: eight band entries of the local slot outcome
 * draw; the entry one past the last is read when the scan exhausts.
 */
extern SlotOutcomeBand D_801F2530[];

/* @source 0x801455CB @kind unknown: outcome byte at +3 of the stride-8 active
 * records; its high nibble carries the 0x10 armed state and its low nibble
 * selects the per-kind record table. */
extern volatile u8 D_801455CB[];
/* @source 0x80145E61 @kind unknown: second byte of the pending notification
 * queue pair selected by the append counter at 0x80145E5D. */
extern volatile u8 D_80145E61[];

/* The four per-kind record tables the draw selects by the low nibble of the
 * record's outcome byte. Each entry is a byte flag at +0 and a halfword value
 * at a table-specific offset, and the record state byte indexes the table.
 */
/* @source 0x801C8970 @kind table */
extern u8 D_801C8970[];
/* @source 0x801C8974 @kind table */
extern u16 D_801C8974[];
/* @source 0x801C90E9 @kind table */
extern u8 D_801C90E9[];
/* @source 0x801C90F2 @kind table */
extern u16 D_801C90F2[];
/* @source 0x801C98B1 @kind table */
extern u8 D_801C98B1[];
/* @source 0x801C98B8 @kind table */
extern u16 D_801C98B8[];
/* @source 0x801C9E89 @kind table */
extern u8 D_801C9E89[];
/* @source 0x801C9E8E @kind table */
extern u16 D_801C9E8E[];

/* @source 0x801EFA50
 * @behavior Resolves the pending battle outcome of every armed local slot
 *           record: it walks the eight stride-8 local record slots at 0x801457A8
 *           carrying slot kind 0xD and, for each, walks the sixty stride-8
 *           active-record entries of 0x801455C8, taking the present entry whose
 *           kind byte equals the slot index plus one and whose +3 outcome byte
 *           carries the 0x10 armed state. The low nibble of that outcome byte
 *           selects one of four per-kind record tables (0x801C8970/0x801C8974,
 *           0x801C90E9/0x801C90F2, 0x801C98B1/0x801C98B8 and
 *           0x801C9E89/0x801C9E8E) and the record state byte indexes it, which
 *           yields a halfword bound and a byte flag; the bound selects the first
 *           of the eight six-byte band entries at 0x801F2530 whose threshold it
 *           does not reach, and the record is skipped unless the shared battle
 *           count at 0x8014502C minus its progress anchor reaches that entry's
 *           requirement. The outcome byte keeps only its low nibble; when the
 *           table flag's bit 3 is set the 0x30 outcome is stored directly, and
 *           otherwise the entry's weight raises a success chance of the weight
 *           plus the weight times the record's stride-9 byte over twenty,
 *           clamped to 0..100 as a signed halfword, which a rand() draw below it
 *           marks with outcome 0x20. A second draw below thirty minus twice that
 *           byte stores outcome 0x40 with record state 0x37, and every other
 *           draw marks outcome 0x30. Each resolved record appends notification
 *           kind 3 and its record index to the pending queue at
 *           0x80145E60/0x80145E61 and advances the queue append byte at
 *           0x80145E5D.
 * @status partial
 * @match 74.10
 * @residual The frame and saved-register set agree. The first mismatch is
 *           the outer slot-skip branch target at +0x38 because the body is
 *           32 bytes short. Record-state indexing adds halfword masks, the
 *           inner-loop setup and latch updates have different order, and
 *           outcome-store tails merge. The chance clamp and random-roll
 *           calculations also differ in branch polarity and value lifetimes.
 *           Complete function bytes do not match.
 */
void resolveSlotBattleOutcomes(void) {
  u8* cell;
  u32 elapsed;
  s32 roll;
  s32 chance;
  s32 band;
  s32 slot;
  s32 slot_off;
  s32 record;
  u16 value;
  u16 need;
  u16 weight;
  u8 flags;
  u8 state;
  s32 off;

  slot = 0;
  slot_off = 0;
  for (; slot < 8; slot++, slot_off += 8) {
    if (D_801457A8[slot_off] != 0xD) {
      continue;
    }
    record = 0;
    off = 0;
    cell = D_801F2708;
    for (; record < 60; record++, off += 8, cell += 9) {
      if (((volatile u8*)activeRecordBytes)[off] == 0) {
        continue;
      }
      if (D_801455C9[off] != slot + 1) {
        continue;
      }
      state = D_801455CB[off];
      if ((state & 0xF0) != 0x10) {
        continue;
      }
      /* off is 0..472 here (480 after the final increment), so the
       * halfword index preserves its value without retyping shared storage. */
      switch (state & 0xF) {
      case 0:
        value = D_801C8974[D_801455CA[(u16)off] * 9];
        flags = D_801C8970[D_801455CA[(u16)off] * 18];
        break;
      case 1:
        value = D_801C90F2[D_801455CA[(u16)off] * 12];
        flags = D_801C90E9[D_801455CA[(u16)off] * 24];
        break;
      case 2:
        value = D_801C98B8[D_801455CA[(u16)off] * 11];
        flags = D_801C98B1[D_801455CA[(u16)off] * 22];
        break;
      default:
        value = D_801C9E8E[D_801455CA[(u16)off] * 10];
        flags = D_801C9E89[D_801455CA[(u16)off] * 20];
        break;
      }
      band = 0;
      while (band < 8 && D_801F2530[band].threshold < (u32)value) {
        band += 1;
      }
      need = D_801F2530[band].requirement;
      elapsed = D_8014502C - *(u32 *)((u8 *)activeRecordBytes + off + 4);
      if (elapsed < need) {
        continue;
      }
      D_801455CB[off] &= 0xF;
      if ((flags & 8) != 0) {
        D_801455CB[off] |= 0x30;
      } else {
        weight = D_801F2530[band].weight;
        chance = weight + weight * *cell / 20;
        if ((s16)chance < 0) {
          chance = 0;
        } else if ((s16)chance > 100) {
          chance = 100;
        }
        roll = rand() & 0x7F;
        if (roll > 100) {
          roll -= 0x1C;
        }
        if ((s16)roll < (s16)chance) {
          D_801455CB[off] |= 0x20;
        } else {
          chance = 30 - *cell * 2;
          roll = rand() & 0x7F;
          if (roll > 100) {
            roll -= 0x1C;
          }
          if ((s16)roll < chance) {
            D_801455CB[off] = 0x40;
            D_801455CA[(u16)off] = 0x37;
          } else {
            D_801455CB[off] |= 0x30;
          }
        }
      }
      ((u8*)D_80145E60)[D_80145E5D * 2] = 3;
      ((u8*)D_80145E61)[D_80145E5D * 2] = record;
      D_80145E5D = D_80145E5D + 1;
    }
  }
}
