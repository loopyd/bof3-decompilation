#include "bof3/ui/game00_internal.h"

/* @source 0x801A7AD8
 * @behavior Walks the count five-byte lookup records at table and returns the
 * sign-extended index of the first record whose key byte matches the pending
 * world id D_80143F00 and whose flags/category gate and coordinate position
 * match the two coordinate arguments; -1 when no record matches.
 * @status partial
 * @match 72.86
 * @residual record-base induction-variable allocation: every clean-C shape
 * tried lets the compiler reassociate the tail accesses onto the parameter base
 * ($a0, key byte at +0) and create the +3 tail base in a fresh register, while
 * the original keeps the +3 tail base in $a0 and copies the parameter into $t3
 * for the key; live audit 51/70 instructions, 280/280 bytes.
 * Live asm-diff first differs at +0x0004: the original's `move $t3,$a0` key-base
 * copy and its `addiu $a0,$a0,3` tail base are replaced by a single `addiu
 * $t2,$a0,3` plus a reassociation of the flag/pair/count loads to +3/+2/+1
 * displacements from $a0. Everything else (count mask and loop entry, world-id
 * load, flag-direction branch layout, both coordinate tests, the low-nibble
 * category gate and the -1 exit) matches modulo register permutation.
 */
s8 func_801A7AD8(u8* table, u8 count, u8 coord_a, u8 coord_b) {
  u8* record;
  s32 i;
  s32 j;
  s32 category;

  record = table + 3;
  for (i = 0; i < count; i++, record += 5) {
    for (j = record[1] - 1; j >= 0; j--) {
      if (record[-3] != D_80143F00) {
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
      if (category == 8) {
        return i;
      }
      if (category != D_80145E98[0].field_00) {
        break;
      }
      return i;
    }
  }
  return -1;
}
