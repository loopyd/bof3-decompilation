#include "bof3/battle/battle15_internal.h"

extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0A28
 * @behavior clears scratchpad work area bytes at offsets 0x00-0x04.
 * @status unverified
 * @match 0.00
 * @residual not measured yet
 */
void clearWorkBytesBmagicMagic00303_801F0A28(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
