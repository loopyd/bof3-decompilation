#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801D4285;
extern u32 D_80144F50;

/* @source 0x801F2C04
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8). While that byte is nonzero it stores 4
 *           into the shared status halfword D_801490A8 and then 6 into the
 *           shared mode index byte D_801D4285. When the gate byte is zero it
 *           tests the main-RAM money word D_80144F50 with an unsigned compare
 *           against 0x2710: below that bound it stores 3 into D_801490A8 and,
 *           through the same join, 6 into D_801D4285; at or above it subtracts
 *           0x2710 from D_80144F50, stores 2 into D_801490A8 and returns
 *           without touching the mode byte. Takes no arguments and returns
 *           nothing; a frameless leaf whose two arm constants (4 and 3) fill
 *           the branch delay slots of the gate test and the money test.
 *           Role evidence: D_80144F50 is the main-RAM money word the shop
 *           overlay subtracts charged prices from and draws in its money panel
 *           (src/bof3/ui/stepShopRowCursorThenChargePriceOrAdvanceUiSubstep.c,
 *           src/bof3/ui/drawShopPriceListPanel.c), and D_801490A8/D_801490C7
 *           are the shared status halfword and gate byte the rest of the
 *           setStatus* world family reads and writes.
 *           Representation: the money word is read through a block-local
 *           pointer, which makes the compiler materialise the address once
 *           (`lui $a0`/`addiu $a0,$a0,%lo`) and reach both the load and the
 *           store through it with zero displacement, as the original does.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateGateMoneyMoney(void) {
  s32 status;

  if (D_801490C7 != 0) {
    status = 4;
  } else {
    u32 *money = &D_80144F50;

    if (*money < 10000u) {
      status = 3;
    } else {
      *money -= 10000;
      D_801490A8 = 2;
      return;
    }
  }
  D_801490A8 = status;
  D_801D4285 = 6;
}
