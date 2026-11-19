#include "bof3/bof3.h"

extern u8 D_80145FA8;
extern u8 D_80146384;

/* @source 0x800C1B50
 * @behavior Overlay handler: when the record state byte at 0x80145FA8 equals
 * 4, publishes 5 into the shared boss state byte at 0x80146384; otherwise
 * returns without touching any state. Entry 0 of this overlay's 0x800C2D18
 * handler table, and byte-identical to entry 0 of the sibling 0x800C2CDC
 * table at 0x800C19B4 and of the 0x800C2D54 table at 0x800C1CA4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setBossStateFiveWhenStateFourBossBoss01816_800C1B50(void) {
  if (D_80145FA8 == 4) {
    D_80146384 = 5;
  }
}
