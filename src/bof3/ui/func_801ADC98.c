#include "bof3/ui/game00_internal.h"

/* @behavior Returns the signed 28-step delta between the work byte at 0x30 and
 * D_80145EC0, scaling negative quotients by four.
 * @source 0x801ADC98
 * @status exact
 * @match 100.00
 * @residual none
 */
s8 func_801ADC98(void) {
  s32 quotient;
  s8 result;

  quotient = (s8)(g_game_work->unk_30 - D_80145EC0);
  quotient /= 28;
  result = (s8)quotient;
  if ((s8)quotient < 0) {
    result = (s8)(quotient * 4);
  }
  return result;
}
