#include "bof3/bof3.h"

extern u8 D_80146375;

/* @source 0x800C19C4
 * @behavior Overlay handler: writes zero to the shared battle state byte at
 * 0x80146375, clearing it, and returns. Entry 0 of this overlay's handler table
 * at 0x800C1D20. Byte-identical to the 0x800C20AC body of BIN/BOSS/BOSS018.EMI#16
 * (payload cmp over the 16-byte span), which measured exact; the name and shape
 * come from that sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearBossStateByteBossBoss02116_800C19C4(void) {
  D_80146375 = 0;
}
