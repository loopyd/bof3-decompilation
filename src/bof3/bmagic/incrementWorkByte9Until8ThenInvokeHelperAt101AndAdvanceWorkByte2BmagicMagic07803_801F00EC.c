#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_8015D71C(u16 arg0); /* @source 0x8015D71C */

/* @source 0x801F00EC
 * @behavior Tests scratchpad work byte 9 against 8. If it is 8, invokes helper
 *           0x8015D71C with 0x101, reloads the work pointer and increments byte
 *           2. Otherwise increments byte 9 without advancing the state. The
 *           pointer cell is 0x1F800044. Takes no arguments and returns nothing;
 *           table word 0x801F08C8 publishes this callback. Helper and work-field
 *           roles remain address- or offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9Until8ThenInvokeHelperAt101AndAdvanceWorkByte2BmagicMagic07803_801F00EC(void) {
  u8 *work = g_battle_work;
  u8 count = work[9];

  if (count == 8) {
    func_8015D71C(0x101);
    g_battle_work[2]++;
  } else {
    work[9] = count + 1;
  }
}
