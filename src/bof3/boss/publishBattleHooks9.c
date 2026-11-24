#include "bof3/bof3.h"

typedef void (*Boss01516Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the state switch
 * at 0x800C1930, which tests bit 1 of D_80146328 and either stores 0x4B into
 * g_ScenarioProgress and runs initModeTuple510 (0x801DDED8), or runs
 * initModeTuple520 (0x801DDF00). The second takes no argument and forwards 1 to
 * func_800AC934. The third returns a zero result, which the void slot type
 * drops. */
void initModeProgress4B(
    void);
void invokeHelperArg(void);
s32  zeroResult11(void);

/* Shared hook slots. 0x801463A4 and 0x801463A8 head the trio every boss overlay
 * of this archive family publishes, and 0x801463AC is the third slot of that
 * same trio. */
extern Boss01516Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss01516Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss01516Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C18F0
 * @behavior Publishes this overlay handler trio into the shared hook slots in
 * slot order: the state switch
 * initModeProgress4B
 * into 0x801463A4, the helper wrapper
 * invokeHelperArg into 0x801463A8, and the zero-result
 * accessor zeroResult11 into 0x801463AC through the
 * void slot type. The 0x38-byte body reserves no frame and keeps nothing across
 * the three stores. No word of this payload names this function and no jal
 * reaches it, so it is an overlay entry reached from outside the payload. Same
 * body as the exact installer at 0x800C18F0 of BIN/BOSS/BOSS029.EMI#16; a
 * payload comparison of the two 0x38-byte spans found one differing word, the
 * second callee's own %lo immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooks9(void) {
  D_801463A4 =
      initModeProgress4B;
  D_801463A8 = invokeHelperArg;
  D_801463AC = (Boss01516Handler)zeroResult11;
}
