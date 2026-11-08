#include "bof3/bof3.h"

extern u8 D_80146328; /* @source 0x80146328 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1F48
 * @behavior Mode-transition handler and first slot of the trio this overlay
 * publishes into the shared battle hook slots. It reads the shared battle flag
 * byte D_80146328 and, when that byte's 0x2 bit is set, stores 5 into the
 * shared mode byte D_80146875 and runs the shared initializer this archive
 * family binds as initModeTuple510 at 0x801DDED8; when the bit is clear it
 * runs initModeTuple520 at 0x801DDF00 instead. It takes no arguments and
 * returns nothing. The installer publishBattleHooksBossBoss00216_800C1F08 of
 * this overlay materializes this function's address and stores it into the
 * shared battle hook slot 0x801463A4; no jal of this payload reaches it, so the
 * main program calls it through that slot. Its body is the same shape as the
 * mode-transition members of the sibling overlays in this family, which differ
 * only in the constant they store into the shared mode byte or in the shared
 * initializer they reach.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or520ByFlagBitTwoBossBoss00216_800C1F48(void) {
  if (D_80146328 & 0x2) {
    D_80146875 = 5;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
