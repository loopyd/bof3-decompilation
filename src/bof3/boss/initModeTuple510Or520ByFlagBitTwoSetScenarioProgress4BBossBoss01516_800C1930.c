#include "bof3/bof3.h"

extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared main-program services this archive family runs at the end of its
 * battle-flag state switch. Fifteen sibling targets of the archive family bind
 * 0x801DDED8 as initModeTuple510 and 0x801DDF00 as initModeTuple520, the two
 * mode-tuple initializers the switch chooses between. */
void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1930
 * @behavior Battle-flag state switch of this overlay and first member of the
 * hook trio its installer publishBattleHooksBossBoss01516_800C18F0 writes. It
 * loads the shared battle flag byte 0x80146328 once, before it reserves its
 * frame, and tests bit 0x2; while the bit is set it stores 0x4B into the shared
 * scenario progress byte 0x80146864 and runs initModeTuple510 (0x801DDED8);
 * when the bit is clear it runs initModeTuple520 (0x801DDF00) instead, and it
 * stores nothing there. It reads no other state, writes only that one byte,
 * takes no arguments, returns nothing, and its 0x18-byte
 * frame keeps $ra at 0x10 across the chosen call. The installer in this payload
 * materializes this function's address into the shared battle-hook slot
 * 0x801463A4; no other word of this payload holds that address and no `jal`
 * reaches it, so the main program reaches it through that slot. Same statement
 * order as the exact sibling
 * initModeTuple510Or520ByFlagBitTwoSetScenarioProgress11BossBoss00816_800C1FDC;
 * a payload comparison of the two 0x4C-byte spans found exactly two differing
 * words, the stored literal and the address encoded in the forward `j`, which
 * shifts by the 0x6AC span delta, so this function stores 0x4B where that one
 * stores 0x11. The carrier
 * initModeTuple510Or520ByFlagBitTwoSetScenarioProgress32BossBoss05416_800C1970
 * sits at 0x800C1970 and differs in those same two words only, its literal 0x32
 * and a `j` shifted by its 0x40 delta.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or520ByFlagBitTwoSetScenarioProgress4BBossBoss01516_800C1930(
    void) {
  if (D_80146328 & 0x2) {
    g_ScenarioProgress = 0x4B;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
