#include "bof3/bof3.h"

extern u8 D_80146255;         /* @source 0x80146255 @kind unknown */
extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared main-program mode-tuple services this archive family runs at the end of
 * its battle-flag state switch: 0x801DDED8 seeds the three-byte battle mode
 * tuple with 5, 1 and 0, and 0x801DDF28 seeds it with 5, 3 and 0. */
void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple530(void); /* @source 0x801DDF28 */

/* @source 0x800C204C
 * @behavior Battle-flag state switch of this overlay and first member of the
 * hook trio its installer writes. It loads the shared battle flag byte
 * 0x80146328 once and tests bit 0x2; while the bit is set it stores 0x32 into
 * the shared scenario progress byte 0x80146864 and runs initModeTuple510
 * (0x801DDED8), which seeds the three-byte battle mode tuple with 5, 1 and 0;
 * when the bit is clear it stores the same 0x32 into 0x80146864, runs
 * initModeTuple530 (0x801DDF28), which seeds that tuple with 5, 3 and 0, and
 * then stores the mode byte 3 into 0x80146255. It reads no other state, writes
 * only those two bytes, takes no arguments and returns nothing, and its
 * 0x18-byte frame keeps $ra at 0x10 across the chosen call. The installer
 * publishBattleHooksBossBoss01316_800C200C next in this payload materializes
 * this function's address into the shared battle-hook slot 0x801463A4; no word
 * of this payload holds that address and no `jal` reaches it, so the main
 * program reaches it through that slot. Same hook slot and condition shape as
 * the exact sibling initModeTuple510Or520ByFlagBitTwoBossBoss00216_800C1F48,
 * which branches on the same bit of the same flag byte into a pair of
 * mode-tuple services; this body differs in the scenario progress value it
 * stores on both paths, in reaching initModeTuple530 instead of
 * initModeTuple520 on the clear path, and in the trailing mode-byte store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or530ByFlagBitTwoSetScenarioProgress32AndModeByte3BossBoss01316_800C204C(
    void) {
  if (D_80146328 & 0x2) {
    g_ScenarioProgress = 0x32;
    initModeTuple510();
  } else {
    g_ScenarioProgress = 0x32;
    initModeTuple530();
    D_80146255 = 0x3;
  }
}
