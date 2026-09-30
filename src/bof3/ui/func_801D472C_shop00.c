#include "bof3/ui/shop00_internal.h"

/* @source 0x801D472C
 * @behavior sub-step handler of the phase table D_801E530C: clears the
 *           overlay-local byte D_801E6110 and arms D_801E60EC with 0x17, emits
 *           the shop panel strip through func_801DAB90 with the fixed panel
 *           arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument, clears D_801E6080, archives the
 *           sub-step byte D_80148652 incremented by one into D_801E6094, then
 *           sets D_80148652 to 0x13 and re-arms phaseTimer with 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D472C(void) {
  u8* step = &D_80148652;
  u8 previous;

  D_801E6110 = 0;
  D_801E60EC = 0x17;
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  D_801E6080 = 0;
  previous = *step;
  *step = 0x13;
  phaseTimer = 2;
  D_801E6094 = previous + 1;
}
