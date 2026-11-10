#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_8015D71C(s32 selector); /* @source 0x8015D71C */

/* @source 0x801EF178
 * @behavior Handler at table word 0x801F08F0. Decrements work byte 0xA
 *           modulo 256 and tests the stored value without reloading it.
 *           On zero, reloads the work pointer and calls helper 0x8015D71C
 *           with 0x101 when byte 0xB is odd, otherwise 0x100. Clears bytes
 *           9 and 0xA and increments byte 2 through separate work-pointer
 *           reloads. Takes no arguments and returns nothing. The helper's
 *           role remains address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAThenInvokeHelperByByteBParityClearBytes9AndAAndAdvanceByte2BmagicMagic10403_801EF178(void) {
  s32 selector;

  if (--g_battle_work[0xA] == 0) {
    selector = 0x100;
    if (g_battle_work[0xB] & 1) {
      selector = 0x101;
    }
    func_8015D71C(selector);
    g_battle_work[9] = 0;
    g_battle_work[0xA] = 0;
    g_battle_work[2]++;
  }
}
