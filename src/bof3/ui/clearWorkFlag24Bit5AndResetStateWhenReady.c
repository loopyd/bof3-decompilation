#include "bof3/ui/game00_internal.h"

/*
 * @source 0x801ABEE0
 * @behavior Clears bit 5 of the scratchpad work-area byte at offset 0x24 and
 * then, when the shared main-executable helper func_8015C148 reports exactly 1,
 * clears the work-area state byte at offset 0x02. The 0x24 byte is unmodelled
 * in struct GameWorkArea, so it is reached through the existing pad_1C[8]
 * spelling already used by src/bof3/ui/func_801A0514.c.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlag24Bit5AndResetStateWhenReady(void) {
  g_game_work->pad_1C[8] &= 0xDF;
  if (func_8015C148() == 1) {
    g_game_work->flags_02 = 0;
  }
}
