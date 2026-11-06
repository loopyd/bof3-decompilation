#include "bof3/bof3.h"

extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */
extern u8 D_80145029;         /* @source 0x80145029 @kind unknown */
extern u8 D_80146255;         /* @source 0x80146255 @kind unknown */
extern u8 D_80146325;         /* @source 0x80146325 @kind unknown */
extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */

void initModeTuple530(void); /* @source 0x801DDF28 */

/* @source 0x800C1A48
 * @behavior Battle-entry hook of this overlay, published into the shared
 * battle-hook slot 0x801463A4 by the installer at 0x800C1A08. It seeds the
 * shared scenario progress byte with 12, stores the entry mode byte 3 into
 * 0x80146255, stores 0x6A into 0x80145029, stores that same mode byte 3 into
 * 0x80146255 a second time, raises bit 0x8 of the shared battle flag byte
 * 0x80146328 through one address taken into a local pointer, clears bit 0x40
 * of 0x80146325 and finally runs initModeTuple530, which seeds the three-byte
 * battle mode tuple. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initBattleEntryStateBossBoss02213_800C1A48(void) {
  u8* flags;

  g_ScenarioProgress = 12;
  D_80146255 = 3;
  D_80145029 = 0x6A;
  D_80146255 = 3;
  flags = &D_80146328;
  *flags |= 8;
  D_80146325 &= 0xBF;
  initModeTuple530();
}
