#include "bof3/ui/game00_internal.h"

/* @behavior Returns the signed 28-step delta between the work byte at 0x30 and
 * D_80145EC0, scaling negative quotients by four.
 * @source 0x801ADC98
 * @status exact
 * @match 100.00
 */
s8 func_801ADC98(void) {
  s32 quotient;
  /* MATCHING_AID: clean C coalesces result into quotient (v1), omitting the
   * original +0x48 a0 copy and conditional result lifetime. */
  REGISTER_PIN(s32, result, "a0");

  quotient = (s8)(g_game_work->unk_30 - D_80145EC0);
  quotient /= 28;
  result = quotient;
  if ((s8)quotient < 0) {
    result = quotient * 4;
  }
  return (s8)result;
}
