#include "bof3/ui/game00_internal.h"

/* @source 0x801A7AD8
 * @behavior Walks the count five-byte lookup records at table and returns the
 * sign-extended index of the first record whose key byte matches the pending
 * world id D_80143F00 and whose flags/category gate and coordinate position
 * match the two coordinate arguments; -1 when no record matches.
 * @status exact
 * @match 100.00
 * @residual none
 */
s8 func_801A7AD8(u8* table, u8 count, u8 coord_a, u8 coord_b) {
  u8* record;
  s32 i;
  s32 j;
  s32 category;

  for (i = 0; i < count; i++, table += 5) {
    record = table + 3;
    for (j = record[1] - 1; j >= 0; j--) {
      if (table[0] != D_80143F00) {
        break;
      }
      if (record[0] & 0x80) {
        if (coord_a != record[-2]) {
          continue;
        }
        if (coord_b == record[-1] + j) {
          goto matched;
        }
        continue;
      }
      if (coord_a != record[-2] + j) {
        continue;
      }
      if (coord_b != record[-1]) {
        continue;
      }
    matched:
      category = record[0] & 0xF;
      if (category != 8) {
        if (category != D_80145E98[0].field_00) {
          break;
        }
      }
      return i;
    }
  }
  return -1;
}
