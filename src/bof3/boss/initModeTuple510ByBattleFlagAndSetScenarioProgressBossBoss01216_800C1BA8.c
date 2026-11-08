#include "bof3/bof3.h"

extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1BA8
 * @behavior Battle-flag state switch of this overlay. It takes the address of
 * the shared battle flag byte 0x80146328 into one local pointer, loads that
 * byte once and tests bit 0x2; while the bit is set it stores 0x23 into the
 * shared scenario progress byte 0x80146864, writes the same loaded value with
 * bit 0x8 raised back to 0x80146328 and runs initModeTuple510 (0x801DDED8),
 * which seeds the three battle-global bytes with 5, 1 and 0; when the bit is
 * clear it runs initModeTuple520 (0x801DDF00) instead, which seeds those bytes
 * with 5, 2 and 0. It reads no other state and writes only those two bytes,
 * takes no arguments and returns nothing, and its 0x18-byte frame keeps $ra at
 * 0x10 across the chosen call. No word of this payload holds this function's
 * address and no `jal` reaches it; the installer at 0x800C1B68 publishes it
 * into the shared battle-hook slot 0x801463A4, so the main program reaches it
 * through that hook. Same condition shape as the exact sibling
 * initModeTupleByBattleFlagAndSetScenarioProgressBossBoss02116_800C1C18 of the
 * boss021/16 overlay; this body differs in storing 0x23 instead of 0xA, in
 * raising bit 0x8 of the flag from the value it already loaded, and in taking
 * the flag through a pointer it reuses for that store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss01216_800C1BA8(
    void) {
  u8* flags;
  u8 value;

  flags = &D_80146328;
  value = *flags;
  if (value & 0x2) {
    g_ScenarioProgress = 0x23;
    *flags = value | 0x8;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
