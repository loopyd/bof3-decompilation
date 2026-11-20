#include "bof3/bof3.h"

typedef void (*Boss04916Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * mode-transition handler: it loads the battle flag byte D_80146328 and, when
 * the 0x2 bit is set, stores 5 into D_80146875 and calls func_801DDED8, and
 * otherwise calls func_801DDF00. The second is an inert handler of this
 * overlay. The third returns a zero result, which the void slot type drops. */
void func_800C1A34(void);
void noopHandlerBossBoss04916_800C1A80(void);
s32  zeroResultBossBoss04916_800C1A2C(void);

/* Shared hook slots, the same trio the later installer of this overlay at
 * 0x800C1A88 overwrites. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss04916Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss04916Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss04916Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C19F4
 * @behavior Publishes this overlay's early battle handler trio into the shared
 * hook slots: the mode-transition handler func_800C1A34 (tests the 0x2 bit of
 * D_80146328 and, when it is set, stores 5 into D_80146875 and calls
 * func_801DDED8, and otherwise calls func_801DDF00) into 0x801463A4, the inert
 * handler noopHandlerBossBoss04916_800C1A80 into 0x801463A8, and the
 * zero-result accessor zeroResultBossBoss04916_800C1A2C into 0x801463AC. Each
 * value is materialized with a lui/addiu address pair and stored through a lui
 * base, so the body returns without reading any state of its own. The later
 * installer of this overlay at 0x800C1A88 replaces the same three slots in the
 * same order with func_800C1AC8 and the closing handlers of the overlay. No
 * in-payload caller: this function's address appears in no word of this payload
 * and no jal reaches it, so the main program reaches it through its overlay
 * entry point. The 0x38 bytes of this span compare equal to the exact lift of
 * publishBattleHooksBossBoss04016_800C19F4 in BIN/BOSS/BOSS040.EMI#16, a
 * payload with the same 0x800C1800 load address, whose every referenced
 * symbol sits at the same address as here, so the body transfers unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss04916_800C19F4(void) {
  D_801463A4 = func_800C1A34;
  D_801463A8 = noopHandlerBossBoss04916_800C1A80;
  D_801463AC = (Boss04916Handler)zeroResultBossBoss04916_800C1A2C;
}
