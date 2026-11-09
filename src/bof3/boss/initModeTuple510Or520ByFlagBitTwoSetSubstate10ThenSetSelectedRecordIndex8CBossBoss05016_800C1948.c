#include "bof3/bof3.h"

/* Shared battle flag byte whose 0x2 bit selects the battle mode this overlay
 * starts in. */
extern u8 D_80146328; /* @source 0x80146328 @kind unknown */

/* Shared secondary sub-state byte. */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

/* Shared EXE selected-record index byte. */
extern u8 D_80145029; /* @source 0x80145029 @kind unknown */

/* Main-program service that seeds the three battle-global bytes with 5, 1, 0. */
void initModeTuple510(void); /* @source 0x801DDED8 */

/* Main-program service that seeds the three battle-global bytes with 5, 2, 0. */
void initModeTuple520(void); /* @source 0x801DDF00 */

/* @source 0x800C1948
 * @behavior State-switch hook and first slot of the handler trio this overlay
 * publishes into the shared battle hook slots. It reads the shared battle flag
 * byte D_80146328 and, when that byte bit 0x2 is set, stores 0x10 into the
 * shared secondary sub-state byte D_80146875, runs initModeTuple510, the
 * main-program service at 0x801DDED8 that seeds the three-byte battle mode
 * tuple with 5, 1 and 0, and then stores 0x8C into the shared EXE
 * selected-record index byte D_80145029; when the bit is clear it runs
 * initModeTuple520 at 0x801DDF00 instead, which seeds the same tuple with 5, 2
 * and 0. It takes no arguments and returns nothing, and its 0x18-byte frame
 * keeps $ra at 0x10 across the chosen call. The installer
 * publishBattleHooksBossBoss05016_800C1908 of this overlay materialized this
 * function address and stored it into the shared battle hook slot 0x801463A4,
 * beside an inert handler and a zero-result accessor; no jal of this payload
 * reaches this function, so the main program calls it through that slot. Its
 * body matched the shape of the mode-transition members of the sibling
 * overlays in this archive family, which differ in the constants they store
 * into the sub-state byte and in whether they also store the selected-record
 * index; the sibling at 0x800C1E7C of BIN/BOSS/BOSS002.EMI#16 stored 0x6D and
 * 0x18 into the scenario progress and selected-record index bytes and then ran
 * initModeTuple510, while this span stores the sub-state byte before the call
 * and the selected-record index after it, and the sibling at 0x800C1DB8 of
 * BIN/BOSS/BOSS036.EMI#16 stored only the sub-state byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or520ByFlagBitTwoSetSubstate10ThenSetSelectedRecordIndex8CBossBoss05016_800C1948(
    void) {
  if (D_80146328 & 0x2) {
    D_80146875 = 0x10;
    initModeTuple510();
    D_80145029 = 0x8C;
  } else {
    initModeTuple520();
  }
}
