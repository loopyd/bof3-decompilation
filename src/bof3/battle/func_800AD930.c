#include "bof3/battle/battle15_internal.h"

/* @source 0x800AD930
 * @behavior Decrements scratchpad work byte 2 while the mode byte at
 * D_801462E6 is not 0xFF; when bit 0 of D_80148624 is clear, advances work
 * byte 1 and clears work byte 2.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * D_801462E6 is deliberately a non-volatile u8: declaring it volatile makes
 * the 0xFF test emit an extra `andi v0,v0,0xff` (32 instructions) and swaps
 * the compare registers, while the original tests the loaded byte directly.
 */
void func_800AD930(void)
{
  if (D_801462E6 != 0xFF) {
    g_battle_work[2]--;
  }
  if ((D_80148624 & 1) == 0) {
    g_battle_work[1]++;
    g_battle_work[2] = 0;
  }
}
