#include "bof3/bof3.h"

extern u8 D_80146328; /* @source 0x80146328 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1A34
 * @behavior Overlay mode-transition handler and first slot of the trio this
 * overlay publishes into the shared battle hook slots. It reads the shared
 * battle flag byte D_80146328 and, when that byte's 0x2 bit is set, stores 5
 * into the shared mode byte D_80146875 and runs initModeTuple510, the service
 * that seeds the three-byte battle mode tuple with 5, 1 and 0; when the bit is
 * clear it runs initModeTuple520 instead, which seeds the same tuple with 5, 2
 * and 0. It takes no arguments and returns nothing. The earlier installer of
 * this overlay at 0x800C19F4 materializes this function's address and stores it
 * into the shared battle hook slot 0x801463A4, beside
 * noopHandlerBossBoss04016_800C1A80 into 0x801463A8 and
 * zeroResultBossBoss04016_800C1A2C into 0x801463AC; no jal in this payload
 * reaches it, so the main program calls it through that slot, and the installer
 * published by publishBattleHooksBossBoss04016_800C1A88 later replaces the same
 * three slots with the 0x26 mode-transition handler. Its body is the same shape
 * as the mode-transition members of the sibling overlays in this family, which
 * differ only in the constant they store into the shared mode byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or520ByFlagBitTwoBossBoss04016_800C1A34(void) {
  if (D_80146328 & 0x2) {
    D_80146875 = 5;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
