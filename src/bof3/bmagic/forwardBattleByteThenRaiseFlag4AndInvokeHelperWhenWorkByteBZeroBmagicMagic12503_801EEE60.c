#include "bof3/bof3.h"

extern void func_800A9820(u8 arg0); /* @source 0x800A9820 @kind function */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

extern u8 D_80146384; /* @source 0x80146384 @kind unknown */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EEE60
 * @behavior Reads the scratchpad work pointer at 0x1F800044 once. When work
 *           byte 0xB is zero, forwards the unsigned battle byte at 0x80146384
 *           to func_800A9820, sets mask 0x4 in the shared battle halfword at
 *           0x801462E8, and calls func_801E5988. Otherwise does nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic12503_801EEE60(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
