#include "bof3/bof3.h"

extern u8 D_80145029;         /* @source 0x80145029 @kind unknown */
extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */
extern u8 D_80146875;         /* @source 0x80146875 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared main-program mode-tuple services that the boss archive family runs at
 * the end of its battle-flag state switch: 0x801DDED8 seeds the three-byte
 * battle mode tuple with 5, 1 and 0, and 0x801DDF00 seeds it with 5, 2 and 0. */
void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1B10
 * @behavior Battle-flag state switch of this overlay and first member of the
 * hook trio its installer writes. It loads the shared battle flag byte
 * 0x80146328 once and tests bit 0x2; while the bit is set it stores 5 into the
 * shared scenario progress byte 0x80146864, stores 0x20 into the shared byte
 * 0x80146875, stores 0x77 into the shared byte 0x80145029 and then runs
 * initModeTuple510 (0x801DDED8); when the bit is clear it runs
 * initModeTuple520 (0x801DDF00) instead. Unlike the sibling switches of this
 * archive family it does not write the flag byte back. It reads no other state,
 * takes no arguments and returns nothing, and its 0x18-byte frame keeps $ra at
 * 0x10 across the chosen call. No word of this payload holds this function
 * address and no `jal` reaches it, so the main program reaches it through the
 * battle hook slot the installer at 0x800C1A34 fills. Same condition and call
 * shape as the exact sibling
 * initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss01216_800C1BA8,
 * which branches on the same bit of the same flag byte into the same pair of
 * mode-tuple services and differs in the flag write-back it adds and in the
 * value it stores into the scenario progress byte. The roles of the shared
 * bytes 0x80146875 and 0x80145029 are not established by this payload, so they
 * stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss03816_800C1B10(
    void) {
  if (D_80146328 & 0x2) {
    g_ScenarioProgress = 5;
    D_80146875 = 0x20;
    D_80145029 = 0x77;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
