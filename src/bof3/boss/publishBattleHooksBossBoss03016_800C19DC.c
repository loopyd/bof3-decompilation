#include "bof3/bof3.h"

typedef void (*Boss03016Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch (it tests the 0x2 bit of 0x80146328), the second is the owner
 * record hook and the third returns a zero result, which the void slot type
 * drops. */
void func_800C1A1C(void);
void stateOwnerHeadsBoss(void);
s32 zeroResultBossBoss03016_800C1A14(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03016Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03016Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03016Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C19DC
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1A1C (state switch: while the 0x2 bit of 0x80146328 is set it
 * increments g_ScenarioProgress, stores 0xC into 0x80146875, calls
 * func_801DDED8 and stores 0x5A into 0x80145029, and otherwise calls
 * func_801DDF00) into 0x801463A4, stateOwnerHeadsBoss
 * (clears the owner record heads for ids 0, 1 and 2 through func_800AC934) into
 * 0x801463A8, and the zero-result accessor zeroResultBossBoss03016_800C1A14
 * into 0x801463AC; the accessor's s32 result is dropped by the void slot type.
 * Same statement order and slot mapping as the exact sibling installers of
 * this archive family, whose bodies differ only in the immediates of the
 * addresses they publish. No in-payload caller: no jal immediate and no
 * pointer word of this payload names 0x800C19DC, so the main program reaches
 * it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss03016_800C19DC(void) {
  D_801463A4 = func_800C1A1C;
  D_801463A8 = stateOwnerHeadsBoss;
  D_801463AC = (Boss03016Handler)zeroResultBossBoss03016_800C1A14;
}
