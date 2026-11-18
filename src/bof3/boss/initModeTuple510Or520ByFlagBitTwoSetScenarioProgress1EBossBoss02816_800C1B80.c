#include "bof3/bof3.h"

extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared main-program mode-tuple services this archive family runs at the end of
 * its battle-flag state switch: 0x801DDED8 seeds the three-byte battle mode tuple
 * with 5, 1 and 0, and 0x801DDF00 seeds it with 5, 2 and 0. */
extern void initModeTuple510(void); /* @source 0x801DDED8 */
extern void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1B80
 * @behavior First member of this overlay's battle-hook trio, the slot its
 * installer publishBattleHooksBossBoss02816_800C1B40 writes into 0x801463A4. It
 * loads the shared battle flag byte 0x80146328 once and tests bit 0x2; while the
 * bit is set it stores 0x1E into the shared scenario progress byte 0x80146864 and
 * runs initModeTuple510 (0x801DDED8), which seeds the three-byte battle mode
 * tuple with 5, 1 and 0; when the bit is clear it runs initModeTuple520
 * (0x801DDF00) instead, which seeds that tuple with 5, 2 and 0, and it stores
 * nothing there. It reads no other state, writes only that one byte, takes no
 * arguments and returns nothing, and its 0x18-byte frame keeps the return address
 * at 0x10 across the chosen call. No other word of this payload holds this
 * function's address and no call reaches it, so the main program reaches it
 * through that slot. Same condition and pair of services as the siblings this
 * archive family already lifted exact, boss008/16's 0x800C1FDC (stores 0x11),
 * 0x800C20C8 (stores 0x20) and 0x800C21C4 (stores 0x30); a payload scan of every
 * extracted binary under out/binaries matched this body's words at ten further
 * spans, each of them differing only in the stored literal and the address its
 * forward jump encodes, and none of them stores 0x1E, so those spans supply the
 * shape and not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or520ByFlagBitTwoSetScenarioProgress1EBossBoss02816_800C1B80(
    void) {
  if (D_80146328 & 0x2) {
    g_ScenarioProgress = 0x1E;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
