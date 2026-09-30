#include "bof3/bof3.h"

extern u8 D_80146375;

/* @source 0x800C20AC
 * @behavior Overlay handler: writes zero to the shared boss state byte at
 * 0x80146375, clearing it, and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearBossStateByteBossBoss01816_800C20AC(void) {
  D_80146375 = 0;
}
