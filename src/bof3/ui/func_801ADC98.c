#include "bof3/ui/game00_internal.h"

/* @behavior Returns the signed 28-step delta between the work byte at 0x30 and
 * D_80145EC0, scaling negative quotients by four.
 * @source 0x801ADC98
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
s8 func_801ADC98(void) {
  s32 quotient;

  s32 result;

  quotient = (s8)(g_game_work->unk_30 - D_80145EC0);
  quotient /= 28;
  result = quotient;
  if ((s8)quotient < 0) {
    result = quotient * 4;
  }
  return (s8)result;
}
