#include "bof3/bof3.h"

/* Volatile party-size byte at 0x801462F0. The original loads it once for the
 * entry test and again on every loop test (`lui`/`lbu` of the same global
 * immediately before the `sltu` bound), so the load is never hoisted. */
extern volatile u8 localBattlerCount;

/* Flags word at the start of the 0x140-byte per-slot flag records based at
 * 0x80145FB8. The original scales the slot index by 0x140 and reads the record
 * word as `lui $at,%hi(D_80145FB8)` + `addu $at,$at,index*0x140` +
 * `lw %lo(D_80145FB8)($at)`; the record form keeps that addressing. */
typedef struct BatlEndFlagRecord {
  volatile u32 flags_00;
  u8           pad_04[0x13c];
} BatlEndFlagRecord;

extern volatile BatlEndFlagRecord D_80145FB8[];

/* The EXE-side per-slot availability test at 0x801DB524 (lifted as
 * isBattlerInactiveOrFlag4000 in the battle/03 target): it reports nonzero
 * when the slot is inactive or carries the 0x4000 rejection flag. Outside
 * this overlay, so it is declared here for its address. */
extern u8 func_801DB524(u8 arg0);

/* @source 0x801EEDA8
 * @behavior Counts the party slots below the volatile count byte at
 *   0x801462F0 that pass both eligibility tests: the per-slot availability
 *   test func_801DB524 reports the slot available (zero), and bit 0x400 of the
 *   flags word at the start of that slot's 0x140-byte record in D_80145FB8 is
 *   clear. The count byte is re-read on every loop test, so the loop stops as
 *   soon as it shrinks below the running index, and the running count is
 *   masked to eight bits only on return. The body runs before the first test,
 *   so a zero count byte skips the loop entirely. The 0x20-byte frame keeps
 *   $ra and $s0-$s2 across the per-slot call; the caller at 0x801EECD0 divides
 *   the work word at 0x8014632C by this count.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 countEligibleBattlersBatlEnd00_801EEDA8(void) {
  u8 index;
  u8 count;

  count = 0;
  index = 0;
  if (localBattlerCount != 0) {
    do {
      if (func_801DB524(index) == 0) {
        if ((D_80145FB8[index].flags_00 & 0x400) == 0) {
          count += 1;
        }
      }
      index += 1;
    } while (index < localBattlerCount);
  }
  return count;
}
