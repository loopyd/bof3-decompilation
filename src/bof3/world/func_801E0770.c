#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E0770
 * @behavior Reports whether the AREA030 work record published at the
 * scratchpad cursor 0x1F800044 sits inside the shared staging window: the
 * check runs only while the 0x80143FC9 mode byte is 4, the 0x801E3208 byte is
 * 0xFF, the 0x80144199 gate byte is clear and the 0x80145E92 phase byte is 3,
 * and it then returns whether the record x at +0x34, y at +0x38 and signed
 * timer at +0x3E are within 0x7FFF, 0x7FFF and 0x400 of the shared reference
 * position 0x80143FFC / 0x80144000 / 0x80144006.
 * @status partial
 * @match 21.82
 * @residual live asm-diff first=+0x0000[o0/c0]: the original opens with
 * `lui $a0,%hi(D_80143FC9); lbu $a0,..; li $v1,4; bne $a0,$v1,..` and a single
 * `addu $v0,$zero,$zero` in that branch's delay slot, while this source emits
 * `lui $v1,..; lbu $v1,..; li $v0,4; bne $v1,$v0,..; move $v0,zero`. The
 * original keeps $v0 reserved by one dominating return-0 value shared by all
 * six failure branches (only the first delay slot materializes it; the other
 * slots hold the 0x3/0x7FFF constants or nop), so its guard bytes take $a0,
 * its guard constants $v1 and the shared window constant $a2; every measured
 * clean-C spelling of the four guards materializes a zero per failure path
 * instead, leaving $v0 free for the guard constants. Instruction count and
 * byte size already agree with the original (55/55, 220/220).
 * Shortest missing evidence: a clean-C spelling or object profile that makes
 * the compiler hoist the single dominating `$v0 = 0` before the guards.
 * Measured negative: all 52 candidates of config/compiler/flag-catalog.json and
 * all four installed historical compilers (gcc-2.6.3/2.8.0/2.8.1/2.95.2-psx)
 * report exact_matches=[] via bin/flag-search (best non-exact 30.51%
 * `-O2 -fno-delayed-branch` is rejected because the original keeps real delay
 * slots). Next untried rung: one bounded
 * `bin/permute emi/world00/area030/04@0x801E0770 --time-limit 60 -j N`.
 * Measured clean-C shapes that all reproduce this same first difference
 * (each reverted to this candidate; writer-attested, not independently
 * re-measured under review): separate `return 0;` guards with a final
 * `return dz < 0x400;` (this file), the same guards with `s32 result = 0;` and
 * a single `return result;`, a `goto fail` / single `fail: return 0;`,
 * `&&`-chained guards with one trailing `return 0;`, fully nested guards with
 * an innermost `if (dz < 0x400) { return 1; } else { return 0; }`, and the
 * `ABS(macro)`/ternary spellings of the three signed magnitudes. Nothing in
 * this file is a matching aid: no pins, clobbers, barriers or empty asm.
 */
s32 func_801E0770(void) {
  Area030WorkRecord* work;
  s32 dx;
  s32 dy;
  s32 dz;

  if (D_80143FC9 != 4) {
    return 0;
  }
  if (D_801E3208 != 0xFF) {
    return 0;
  }
  if (D_80144199[0] != 0) {
    return 0;
  }
  if (D_80145E92 != 3) {
    return 0;
  }
  work = (Area030WorkRecord*)D_1F800044;
  dx = work->x_34 - D_80143FFC;
  if (dx < 0) {
    dx = -dx;
  }
  if (dx > 0x7FFF) {
    return 0;
  }
  dy = work->y_38 - D_80144000;
  if (dy < 0) {
    dy = -dy;
  }
  if (dy > 0x7FFF) {
    return 0;
  }
  dz = work->counter_3E - D_80144006;
  if (dz < 0) {
    dz = -dz;
  }
  return dz < 0x400;
}
