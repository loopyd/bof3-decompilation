#include "bof3/bof3.h"

typedef void (*Boss01716Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 0x14 into g_ScenarioProgress and calls func_801DDED8, and otherwise calls
 * func_801DDF00. The second calls func_800AC844 with argument zero. The third
 * returns a zero result, which the void slot type drops. */
void func_800C1A04(void);
void callZeroArgHook(void);
s32  zeroResult23(void);

/* Shared hook slots. 0x801463A4 is the battle hook the battle step loads and
 * calls (jalr) at the end of its update; 0x801463A8 is the hook the selection
 * stream starter calls. 0x801463AC is the third slot of the same trio, written
 * by every boss overlay of this archive family; nothing in the main code of
 * this tree reads it. */
extern Boss01716Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss01716Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss01716Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C19C4
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1A04 (state switch: tests the 0x2 bit of D_80146328 and, while it is
 * set, stores 0x14 into g_ScenarioProgress and calls func_801DDED8, and
 * otherwise calls func_801DDF00) into 0x801463A4,
 * callZeroArgHook (calls func_800AC844(0)) into 0x801463A8,
 * and the zero-result accessor zeroResult23 into
 * 0x801463AC; the accessor's s32 result is dropped by the void slot type. Same
 * trio shape as the sibling installers of this archive family, which write the
 * same three slots in the same order. No in-payload caller: this function's
 * address appears in no other word of this payload and no jal reaches it, so
 * the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksMode14(void) {
  D_801463A4 = func_800C1A04;
  D_801463A8 = callZeroArgHook;
  D_801463AC = (Boss01716Handler)zeroResult23;
}
