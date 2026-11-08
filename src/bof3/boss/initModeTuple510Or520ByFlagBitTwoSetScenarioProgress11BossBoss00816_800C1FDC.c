#include "bof3/bof3.h"

extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared main-program mode-tuple services this archive family runs at the end of
 * its battle-flag state switch: 0x801DDED8 seeds the three-byte battle mode tuple
 * with 5, 1 and 0, and 0x801DDF00 seeds it with 5, 2 and 0. */
void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1FDC
 * @behavior Battle-flag state switch of this overlay and first member of the
 * hook trio its installer publishBattleHooksBossBoss00816_800C1F9C writes. It
 * loads the shared battle flag byte 0x80146328 once and tests bit 0x2; while the
 * bit is set it stores 0x11 into the shared scenario progress byte 0x80146864
 * and runs initModeTuple510 (0x801DDED8), which seeds the three-byte battle mode
 * tuple with 5, 1 and 0; when the bit is clear it runs initModeTuple520
 * (0x801DDF00) instead, which seeds that tuple with 5, 2 and 0, and it stores
 * nothing there. It reads no other state, writes only that one byte, takes no
 * arguments and returns nothing, and its 0x18-byte frame keeps $ra at 0x10
 * across the chosen call. The installer next in this payload materializes this
 * function's address into the shared battle-hook slot 0x801463A4; no other word
 * of this payload holds that address and no `jal` reaches it, so the main
 * program reaches it through that slot. Same condition and pair-of-services
 * shape as the exact sibling initModeTuple510Or520ByFlagBitTwoSetScenarioProgress32BossBoss05416_800C1970,
 * whose 0x4C-byte body differs in exactly two words: the literal it stores and
 * the address encoded in its forward `j`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or520ByFlagBitTwoSetScenarioProgress11BossBoss00816_800C1FDC(
    void) {
  if (D_80146328 & 0x2) {
    g_ScenarioProgress = 0x11;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
