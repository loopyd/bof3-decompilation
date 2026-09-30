#include "bof3/world/area02713_internal.h"

/* @behavior Seeds one 0x98-byte world work record's projected trail from its
 * fixed-point position/rotation pair: the x and y components add the record
 * radius scaled by the two 12-bit angular helpers to the record centre word
 * pair, z is the record word at 0x08 in 8-bit fixed point, and the projected
 * entry 0 of the record trail is then copied into the remaining 31 entries.
 * @source 0x801F304C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F304C(void* arg0) {
  World00Area027Work* work;
  VECTOR              point;
  u8                  scratch[0x20];
  u8                  i;

  work = (World00Area027Work*)arg0;

  point.vx = work->unk_00 +
             (((s32)work->unk_10 * func_801783C8(work->unk_14)) >> 12);
  point.vy = work->unk_04 +
             (((s32)work->unk_10 * func_801782FC(work->unk_14)) >> 12);
  point.vz = (s32)work->unk_08 << 8;

  func_801AFE18(scratch);
  func_801AFF04(&point, work->trail_18);

  i = 1u;
  do {
    work->trail_18[i] = work->trail_18[0];
    i += 1u;
  } while (i < 0x20u);
}
