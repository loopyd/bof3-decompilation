#include "bof3/ui/game00_internal.h"

/* @behavior runs the palette-region stage helper func_801C2438, then resets the
 * active scratch work record: the three signed tint channels at 0x5D/0x5E/0x5F
 * are set to the -0x80 floor, the flags byte at 0x5C to 1, byte 0x12B of the
 * record published at D_80146250 to 7, bit 0x40 of the work flags byte at 0x00
 * is cleared, and the work handler index byte at 0x03 is advanced to 1.
 * @source 0x801B72B0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801B72B0(void) {
  func_801C2438();
  g_game_work->unk_5D = -0x80;
  g_game_work->unk_5E = -0x80;
  g_game_work->unk_5F = -0x80;
  g_game_work->flags_5C = 1;
  D_80146250[0x12B] = 7;
  g_game_work->flags_00 &= 0xBF;
  g_game_work->pad_03 = 1;
}
