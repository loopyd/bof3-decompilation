#include "bof3/bof3.h"

typedef void (*Boss03416Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * mode-transition handler: it tests the 0x2 bit of D_80146328 and, while it is
 * set, stores 5 into the byte at D_80146875 and calls initModeTuple510, and
 * otherwise calls initModeTuple520. The second is an inert handler that returns
 * immediately. The third returns a zero result, which the void slot type
 * drops. */
void initModeTupleByBattleFlagBossBoss03416_800C2380(void);
void noopHandlerBossBoss03416_800C23CC(void);
s32  zeroResultBossBoss03416_800C2378(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03416Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03416Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03416Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C2340
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * initModeTupleByBattleFlagBossBoss03416_800C2380 (mode transition: tests the
 * 0x2 bit of D_80146328 and, while it is set, stores 5 into the byte at
 * D_80146875 and calls initModeTuple510, and otherwise calls initModeTuple520)
 * into 0x801463A4, the inert handler noopHandlerBossBoss03416_800C23CC into
 * 0x801463A8, and the zero-result accessor zeroResultBossBoss03416_800C2378
 * into 0x801463AC; the accessor's s32 result is dropped by the void slot type.
 * Same trio shape as the sibling installers of this archive family, which write
 * the same three slots in the same order and differ only in the published
 * addresses. No in-payload caller: neither this function's address nor the
 * three published addresses appear as a word of this payload and no jal
 * reaches it, so the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss03416_800C2340(void) {
  D_801463A4 = initModeTupleByBattleFlagBossBoss03416_800C2380;
  D_801463A8 = noopHandlerBossBoss03416_800C23CC;
  D_801463AC = (Boss03416Handler)zeroResultBossBoss03416_800C2378;
}
