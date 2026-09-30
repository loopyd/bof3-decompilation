#include "bof3/world/area02713_internal.h"

/* @behavior shifts the 32-entry projected trail at 0x18 of one 0x98-byte world
 * work record up by one entry, then projects the record's rotated position into
 * the freed entry 0: the x and y components add the record radius scaled by the
 * two 12-bit angular helpers to the record centre word pair, z is the record
 * word at 0x08 in 8-bit fixed point.
 * @source 0x801F2E3C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2E3C(void* arg0) {
  World00Area027Work* work;
  VECTOR              point;
  u8                  scratch[0x20];
  u8                  i;

  work = (World00Area027Work*)arg0;

  i = 0x1Fu;
  do {
    work->trail_18[i] = work->trail_18[i - 1];
    i -= 1u;
  } while (i != 0u);

  point.vx = work->unk_00 +
             (((s32)work->unk_10 * func_801783C8(work->unk_14)) >> 12);
  point.vy = work->unk_04 +
             (((s32)work->unk_10 * func_801782FC(work->unk_14)) >> 12);
  point.vz = (s32)work->unk_08 << 8;

  func_801AFE18(scratch);
  func_801AFF04(&point, work->trail_18);
}
