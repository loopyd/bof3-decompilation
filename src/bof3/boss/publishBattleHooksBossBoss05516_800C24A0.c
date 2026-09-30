#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

/* Overlay entrypoints this installer publishes; the first returns void, the
 * second calls the main-program helper at 0x800AC844 with argument zero, and
 * the third is the overlay's zero-result accessor. */
void func_800C24E0(void);
void invokeZeroArgBossBoss05516_800C26B4(void);
s32  zeroResultBossBoss05516_800C24D8(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss05516Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss05516Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss05516Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C24A0
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * the overlay handler 0x800C24E0 into 0x801463A4, the thin wrapper
 * invokeZeroArgBossBoss05516_800C26B4 (calls 0x800AC844 with argument zero)
 * into 0x801463A8, and the zero-result accessor
 * zeroResultBossBoss05516_800C24D8 into 0x801463AC; the accessor is stored
 * through the void slot type, so its s32 result is never consumed. No
 * in-payload caller: the main program reaches it through its overlay entry
 * point, as it does the sibling installer 0x800C1F9C of
 * BIN/BOSS/BOSS008.EMI#16, which writes the same three slots.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss05516_800C24A0(void) {
  D_801463A4 = func_800C24E0;
  D_801463A8 = invokeZeroArgBossBoss05516_800C26B4;
  D_801463AC = (Boss05516Handler)zeroResultBossBoss05516_800C24D8;
}
