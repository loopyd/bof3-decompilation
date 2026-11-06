#include "bof3/bof3.h"

typedef void (*Boss02416Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 5 into g_ScenarioProgress, stores 0x44 into D_80145029, sets the 0x8 bit of
 * D_80146328 and calls func_801DDED8, and otherwise calls func_801DDF00. The
 * second calls the main-program helper at 0x800AC934 with argument 3; the third
 * returns a zero result, which the void slot type drops. */
void func_800C1A28(void);
void invokeHelperArgBossBoss02416_800C1A90(void);
s32  zeroResultBossBoss02416_800C1A20(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02416Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02416Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02416Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C19E8
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1A28 (state switch: tests the 0x2 bit of D_80146328 and, while it is
 * set, stores 5 into g_ScenarioProgress, stores 0x44 into D_80145029, sets the
 * 0x8 bit of D_80146328 and calls func_801DDED8, and otherwise calls
 * func_801DDF00) into 0x801463A4, the thin wrapper
 * invokeHelperArgBossBoss02416_800C1A90 (calls 0x800AC934 with argument 3) into
 * 0x801463A8, and the zero-result accessor
 * zeroResultBossBoss02416_800C1A20 into 0x801463AC; the accessor's s32 result is
 * dropped by the void slot type. Same trio shape as the sibling installers of
 * this archive family, which write the same three slots in the same order. No
 * in-payload caller: neither this function's address nor the three slot
 * addresses appear in any other word of this payload, and no jal reaches it, so
 * the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss02416_800C19E8(void) {
  D_801463A4 = func_800C1A28;
  D_801463A8 = invokeHelperArgBossBoss02416_800C1A90;
  D_801463AC = (Boss02416Handler)zeroResultBossBoss02416_800C1A20;
}
