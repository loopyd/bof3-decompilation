#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF284
 * @behavior Reads the scratchpad work pointer once and, when work byte 0xB
 *           is zero, raises mask 0x4 in the halfword at 0x801462E8 and invokes
 *           0x801E5988 with no arguments. Returns nothing. Published at
 *           payload offset 0x14 in the overlay's leading pointer run.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic06703_801EF284(void) {
  u8 *work = g_battle_work;

  if (work[0xB] == 0) {
    u16 *flags = (u16 *)&D_801462E8;

    *flags |= 4;
    func_801E5988();
  }
}
