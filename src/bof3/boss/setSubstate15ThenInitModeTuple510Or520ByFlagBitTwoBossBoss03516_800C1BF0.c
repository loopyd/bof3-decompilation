#include "bof3/bof3.h"

extern u8 D_80146328; /* @source 0x80146328 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1BF0
 * @behavior Overlay state-switch hook and first slot of the trio this overlay
 * publishes into the shared battle hook slots. It reads the shared battle flag
 * byte D_80146328 and, when that byte bit 0x2 is set, stores 0x15 into the
 * shared front-end secondary sub-state byte D_80146875 and runs
 * initModeTuple510, the main-program service at 0x801DDED8 that seeds the
 * three-byte battle mode tuple with 5, 1 and 0; when the bit is clear it runs
 * initModeTuple520 at 0x801DDF00 instead, which seeds the same tuple with 5, 2
 * and 0. It takes no arguments and returns nothing. The installer
 * publishBattleHooksBossBoss03516_800C1B4C materializes this function address
 * and stores it into the shared battle hook slot 0x801463A4; no jal of this
 * payload reaches it, so the main program calls it through that slot. Its body
 * is the same shape as the mode-transition members of the sibling overlays in
 * this archive family, which differ only in the constant they store into the
 * shared sub-state byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate15ThenInitModeTuple510Or520ByFlagBitTwoBossBoss03516_800C1BF0(
    void) {
  if (D_80146328 & 0x2) {
    D_80146875 = 0x15;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
