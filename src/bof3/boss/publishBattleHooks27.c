#include "bof3/bof3.h"

typedef void (*Boss05416Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 0x32 into g_ScenarioProgress and calls func_801DDED8, and otherwise calls
 * func_801DDF00. The second is an inert handler; the third returns a zero
 * result, which the void slot type drops. */
void initModeProgress32(void);
void noopHandler106(void);
s32  zeroResult(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss05416Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss05416Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss05416Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1930
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * initModeProgress32
 * (state switch: tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 0x32 into g_ScenarioProgress and calls func_801DDED8, and otherwise calls
 * func_801DDF00) into 0x801463A4, the inert
 * noopHandler106 into 0x801463A8, and the zero-result
 * accessor zeroResult into 0x801463AC; the accessor's s32
 * result is dropped by the void slot type. Same trio shape as the sibling
 * installers of this archive family, which write the same three slots in the
 * same order. No in-payload caller: neither this function's address nor the
 * three slot addresses appear in any other word of this payload, and no jal
 * reaches it, so the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooks27(void) {
  D_801463A4 = initModeProgress32;
  D_801463A8 = noopHandler106;
  D_801463AC = (Boss05416Handler)zeroResult;
}
