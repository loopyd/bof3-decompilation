#include "bof3/bof3.h"

typedef void (*Boss04016Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * mode-transition handler: it loads the battle flag byte D_80146328, and while
 * its 0x2 bit is set stores 0x26 into D_80146875 and calls func_801DDED8, and
 * otherwise calls func_801DDF00. The second is an inert handler of this
 * overlay. The third returns a zero result, which the void slot type drops. */
void func_800C1AC8(void);
void noopHandlerBossBoss04016_800C1B14(void);
s32  zeroResultBossBoss04016_800C1AC0(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss04016Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss04016Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss04016Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1A88
 * @behavior Publishes this overlay's late-phase handler trio into the shared
 * hook slots: func_800C1AC8 (mode-transition handler: tests the 0x2 bit of
 * D_80146328 and, while it is set, stores 0x26 into D_80146875 and calls
 * func_801DDED8, and otherwise calls func_801DDF00) into 0x801463A4, the inert
 * handler noopHandlerBossBoss04016_800C1B14 into 0x801463A8, and the
 * zero-result accessor zeroResultBossBoss04016_800C1AC0 into 0x801463AC; the
 * accessor's s32 result is dropped by the void slot type. Same trio shape as
 * the sibling installers of this archive family, which write the same three
 * slots in the same order, and as the earlier installer of this overlay at
 * 0x800C19F4, which replaces the same three slots with func_800C1A34,
 * noopHandlerBossBoss04016_800C1A80 and zeroResultBossBoss04016_800C1A2C. No
 * in-payload caller: this function's address appears in no other word of this
 * payload and no jal reaches it, so the main program reaches it through its
 * overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss04016_800C1A88(void) {
  D_801463A4 = func_800C1AC8;
  D_801463A8 = noopHandlerBossBoss04016_800C1B14;
  D_801463AC = (Boss04016Handler)zeroResultBossBoss04016_800C1AC0;
}
