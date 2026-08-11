#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A3A10
 * @behavior Computes the selection success threshold from battle globals and
 * compares it with a random percentage roll.
 * @status partial
 * @match 95.40
 * @residual early-return branch uses a nop delay slot plus an extra jump;
 * result-lifetime variants regressed and one bounded permuter run found no exact.
 */
s32 func_800A3A10(s32 battler_index, s32 selection_kind)
{
  s32 upper;
  s32 lower;
  s32 upper_base;
  s16 modifier;

  (void)battler_index;
  upper_base = D_801EC312;
  upper = (upper_base / 5) + 50;
  if (upper >= 101) {
    upper = 100;
  }

  lower = D_801EC2F2;
  lower = 125 - (lower / 5);
  if (lower < 50) {
    lower = 50;
  }

  modifier = func_800A2D70(selection_kind & 0xFF,
                           D_801CA71C[D_801463C0].mask & 0x1FF);
  if (modifier == -1) {
    return 0;
  }

  return (rand() % 100) >= ((upper * lower * modifier) / 10000);
}
