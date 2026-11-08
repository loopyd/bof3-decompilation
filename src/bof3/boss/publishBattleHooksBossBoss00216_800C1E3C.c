#include "bof3/bof3.h"

typedef void (*Boss00216Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the raw-address
 * routine at 0x800C1E7C, which tests the 0x6 mask of D_80146328 and either
 * calls func_800AC870(0, D_801EB630, 0), stores 0x6D into g_ScenarioProgress
 * and 0x18 into D_80145029 and then calls func_801DDED8, or calls
 * func_801DDF00. The second takes no argument and forwards zero to
 * func_800AC844. The third returns a zero result, which the void slot type
 * drops. */
void func_800C1E7C(void);
void invokeZeroArgBossBoss00216_800C1EE8(void);
s32 zeroResultBossBoss00216_800C1E74(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss00216Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss00216Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss00216Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1E3C
 * @behavior Publishes this overlay's handler trio into the shared hook slots
 * in slot order: the progress hook at 0x800C1E7C into 0x801463A4, the
 * zero-argument wrapper invokeZeroArgBossBoss00216_800C1EE8 into 0x801463A8,
 * and the zero-result accessor zeroResultBossBoss00216_800C1E74 into
 * 0x801463AC, the last one through the void slot type. No payload word names
 * this function and no jal reaches it, so the main program enters it through
 * the overlay entry point, as it does the sibling installers 0x800C1B0C of
 * BIN/BOSS/BOSS052.EMI#16, 0x800C1D70 of BIN/BOSS/BOSS033.EMI#16 and
 * 0x800C1E50 of BIN/BOSS/BOSS001.EMI#13, which write the same three slots in
 * the same order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss00216_800C1E3C(void) {
  D_801463A4 = func_800C1E7C;
  D_801463A8 = invokeZeroArgBossBoss00216_800C1EE8;
  D_801463AC = (Boss00216Handler)zeroResultBossBoss00216_800C1E74;
}
