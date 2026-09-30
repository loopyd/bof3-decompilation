#include "bof3/bof3.h"

extern u8 D_80145FA8;
extern u8 D_80146384;

/* @source 0x800C1CA4
 * @behavior Overlay handler: when the record state byte at 0x80145FA8 equals
 * 4, publishes 5 into the shared boss state byte at 0x80146384; otherwise
 * returns without touching any state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setBossStateFiveWhenStateFourBossBoss01816_800C1CA4(void) {
  if (D_80145FA8 == 4) {
    D_80146384 = 5;
  }
}
