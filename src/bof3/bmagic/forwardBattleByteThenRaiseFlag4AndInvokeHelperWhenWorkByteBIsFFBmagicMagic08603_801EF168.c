#include "bof3/bof3.h"

void func_800A9820(u8 arg0);
void func_801E5988(void);

extern u8 D_80146384; /* @source 0x80146384 @kind unknown */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x801EF168
 * @behavior Reads the scratchpad work pointer at 0x1F800044 once. When
 *           work byte 0xB equals 0xFF, forwards the shared battle byte at
 *           0x80146384 to func_800A9820, sets mask 0x4 in the shared halfword
 *           at 0x801462E8, then invokes func_801E5988. Otherwise returns
 *           without a call or store. Takes no arguments and returns nothing;
 *           the payload contains its address at 0x801EEC18.
 *           Statement order and access forms follow magic121/03@0x801EEE08:
 *           cmp of the original payload spans at offsets 0x568 and 0x208
 *           proved all 0x60 bytes equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBIsFFBmagicMagic08603_801EF168(void) {
  u8 *work = SPAD_PTR_SLOT(u8, 0x44u);

  if (work[0xB] == 0xFF) {
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
