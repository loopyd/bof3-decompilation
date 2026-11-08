#include "bof3/bof3.h"

extern u8 D_80146328; /* @source 0x80146328 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1E4C
 * @behavior Overlay state-switch hook and first slot of the trio this overlay
 * publishes into the shared battle hook slots. It reads the shared battle flag
 * byte D_80146328 and, when that byte's 0x2 bit is set, stores 5 into the
 * shared secondary sub-state byte D_80146875 and runs initModeTuple510, the
 * main-program service at 0x801DDED8 that seeds the three-byte battle mode
 * tuple with 5, 1 and 0; when the bit is clear it runs initModeTuple520 at
 * 0x801DDF00 instead, which seeds the same tuple with 5, 2 and 0. It takes no
 * arguments and returns nothing. The installer
 * publishBattleHooksBossBoss03616_800C1E0C materializes this function's address
 * and stores it into the shared battle hook slot 0x801463A4, beside
 * noopHandlerBossBoss03616_800C1E98 into 0x801463A8 and
 * zeroResultBossBoss03616_800C1E44 into 0x801463AC; no jal in this payload
 * reaches it, so the main program calls it through that slot, and it replaces
 * the same first slot with the earlier mode-transition handler
 * initModeTuple510Or520ByFlagBitTwoAndSetSubstate8BossBoss03616_800C1DB8 that
 * func_800C1D78 installs. Its body is the same shape as the mode-transition
 * members of the sibling overlays in this archive family, which differ only in
 * the constant they store into the shared sub-state byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or520ByFlagBitTwoAndSetSubstate5BossBoss03616_800C1E4C(
    void) {
  if (D_80146328 & 0x2) {
    D_80146875 = 5;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
