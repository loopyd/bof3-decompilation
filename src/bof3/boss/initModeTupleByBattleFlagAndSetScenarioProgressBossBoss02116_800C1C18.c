#include "bof3/bof3.h"

extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1C18
 * @behavior Mode-transition handler of this overlay. It reads bit 0x2 of the
 * shared battle flag byte 0x80146328 and, while that bit is set, stores 0xA into
 * the shared scenario progress byte 0x80146864 and runs initModeTuple510, the
 * main-program service that seeds the three battle-global bytes with 5, 1 and
 * 0; when the bit is clear it runs initModeTuple520 instead, which seeds the
 * same three bytes with 5, 2 and 0. It takes no arguments and returns nothing,
 * and its 0x18-byte frame only keeps $ra across the chosen call. No word of
 * this payload holds this function's address and no `jal` reaches it, so the
 * main program reaches it through the overlay entry point. It is the
 * 0x4C-byte mode-transition body this boss archive family shares: instruction
 * for instruction identical to the exact sibling at 0x800C2380 of the
 * boss034/16 overlay apart from the byte it stores and that byte's constant.
 * Prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTupleByBattleFlagAndSetScenarioProgressBossBoss02116_800C1C18(
    void) {
  if (D_80146328 & 0x2) {
    g_ScenarioProgress = 0xA;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
