#include "bof3/bof3.h"

/* Shared battle flag word whose low byte carries the two mode bits this hook
 * tests. The original loads it as a word and masks 0x6, unlike the sibling
 * 0x800C1F48, which loads the same cell as a byte and masks 0x2. */
extern u32 D_80146328; /* @source 0x80146328 @kind unknown */

/* Shared scenario progress byte. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* Shared EXE selected-record index byte. */
extern u8 D_80145029; /* @source 0x80145029 @kind unknown */

/* Shared battle record base the main program keeps the current battle records
 * in; this hook passes its address as the copy source. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind unknown */

/* Shared main-program copy of one field group of a caller-supplied record into
 * the battle record the owner id selects, choosing the group by mode. */
void copyOwnerRecordFieldsByMode(u8 id, u8 *source, u8 mode); /* @source 0x800AC870 */

/* Main-program service that seeds the three battle-global bytes with 5, 1, 0. */
void initModeTuple510(void); /* @source 0x801DDED8 */

/* Main-program service that seeds the three battle-global bytes with 5, 2, 0. */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1E7C
 * @behavior Progress hook of this overlay. The installer
 * publishBattleHooksBossBoss00216_800C1E3C stores this function's address into
 * the shared battle hook slot 0x801463A4, so the main program reaches it
 * through that slot; no word of this payload holds the address and no jal
 * reaches it. It reads the shared battle flag word D_80146328 and tests the
 * 0x6 mask of its low byte: while the mask is set it copies the owner-0 field
 * group of the shared battle record base D_801EB630 with
 * copyOwnerRecordFieldsByMode(0, D_801EB630, 0), stores 0x6D into the shared
 * scenario progress byte g_ScenarioProgress, stores 0x18 into the shared
 * selected-record index byte D_80145029 and runs initModeTuple510 at
 * 0x801DDED8, which seeds the three battle-global bytes with 5, 1 and 0; when
 * the mask is clear it runs initModeTuple520 at 0x801DDF00 instead, which seeds
 * those bytes with 5, 2 and 0. It takes no arguments and returns nothing, and
 * its 0x18-byte frame keeps $ra at 0x10 across the chosen call. The 0x6C-byte
 * body carries the statement order of the still-asm sibling at 0x800C1DD4 of
 * this same overlay, which stores 0x24 and 0x1B and reaches a one-argument
 * helper, and of the exact sibling
 * initModeTuple510Or520ByFlagBitTwoBossBoss00216_800C1F48, which reads the same
 * flag as a byte and tests bit 0x2. Prologue and return path place the
 * boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510ByBattleFlagMask6CopyOwnerFieldsAndSetScenarioProgress6DBossBoss00216_800C1E7C(
    void) {
  if (D_80146328 & 0x6) {
    copyOwnerRecordFieldsByMode(0, D_801EB630, 0);
    g_ScenarioProgress = 0x6D;
    D_80145029 = 0x18;
    initModeTuple510();
  } else {
    initModeTuple520();
  }
}
