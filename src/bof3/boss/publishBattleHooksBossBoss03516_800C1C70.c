#include "bof3/bof3.h"

typedef void (*Boss03516Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * alternate state switch: it tests the 0x2 bit of the byte at D_80146328 and,
 * while it is set, stores 0x5 into the byte at D_80146875 and calls
 * func_801DDED8, and otherwise calls func_801DDF00. The second is an inert
 * handler that returns immediately. The third is the zero-result accessor. */
void func_800C1CB0(void);
void noopHandlerBossBoss03516_800C1CFC(void);
s32  zeroResultBossBoss03516_800C1CA8(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03516Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03516Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03516Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1C70
 * @behavior Publishes this overlay's second handler trio into the shared hook
 * slots: func_800C1CB0 (alternate state switch: tests the 0x2 bit of the byte
 * at D_80146328 and, while it is set, stores 0x5 into the byte at D_80146875
 * and calls func_801DDED8, and otherwise calls func_801DDF00) into 0x801463A4,
 * the inert handler noopHandlerBossBoss03516_800C1CFC into 0x801463A8, and the
 * zero-result accessor zeroResultBossBoss03516_800C1CA8 into 0x801463AC; the
 * accessor is stored through the slot type, so its s32 result is never
 * consumed. Same trio shape and statement order as the sibling installer
 * publishBattleHooksBossBoss03516_800C1B4C, which writes the same three slots
 * for this overlay's first trio; only the three published addresses differ. No
 * in-payload caller: neither this function's address nor the three published
 * addresses appear as a word of this payload and no jal reaches it, so the main
 * program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss03516_800C1C70(void) {
  D_801463A4 = func_800C1CB0;
  D_801463A8 = noopHandlerBossBoss03516_800C1CFC;
  D_801463AC = (Boss03516Handler)zeroResultBossBoss03516_800C1CA8;
}
