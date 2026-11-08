#include "bof3/bof3.h"

typedef void (*Boss01216Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 0x23 into g_ScenarioProgress, sets the 0x8 bit of D_80146328 and calls
 * func_801DDED8, and otherwise calls func_801DDF00. The second is an inert
 * handler; the third returns a zero result, which the void slot type drops. */
void initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss01216_800C1BA8(
    void);
void noopHandlerBossBoss01216_800C1C00(void);
s32  zeroResultBossBoss01216_800C1BA0(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss01216Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss01216Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss01216Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1B68
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss01216_800C1BA8
 * (state switch: tests the 0x2 bit of D_80146328 and, while it
 * is set, stores 0x23 into g_ScenarioProgress, sets the 0x8 bit of D_80146328
 * and calls func_801DDED8, and otherwise calls func_801DDF00) into
 * 0x801463A4, the inert noopHandlerBossBoss01216_800C1C00 into 0x801463A8,
 * and the zero-result accessor zeroResultBossBoss01216_800C1BA0 into
 * 0x801463AC; the accessor's s32 result is dropped by the void slot type. Same
 * trio shape as the sibling installers of this archive family, which write the
 * same three slots in the same order. No in-payload caller: neither this
 * function's address nor the three slot addresses appear in any other word of
 * this payload, and no jal reaches it, so the main program reaches it through
 * its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss01216_800C1B68(void) {
  D_801463A4 =
      initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss01216_800C1BA8;
  D_801463A8 = noopHandlerBossBoss01216_800C1C00;
  D_801463AC = (Boss01216Handler)zeroResultBossBoss01216_800C1BA0;
}
