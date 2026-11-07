#include "bof3/bof3.h"

typedef void (*Boss02816Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch at 0x800C1B80; the second is the thin wrapper that calls the
 * main-program helper at 0x800AC934 with argument zero; the third returns a
 * zero result, which the void slot type drops. */
void func_800C1B80(void);
void invokeZeroArgBossBoss02816_800C1BCC(void);
s32  zeroResultBossBoss02816_800C1B78(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02816Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02816Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02816Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1B40
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * the state switch at 0x800C1B80 into 0x801463A4, the thin wrapper
 * invokeZeroArgBossBoss02816_800C1BCC (calls 0x800AC934 with argument zero)
 * into 0x801463A8, and the zero-result accessor
 * zeroResultBossBoss02816_800C1B78 into 0x801463AC; the accessor's s32 result
 * is dropped by the void slot type. Same trio shape as the sibling installers
 * of this archive family, which store the same three slots in the same order.
 * No in-payload caller: neither this function's address nor the three slot
 * addresses appear in any other word of this payload, and no jal reaches it,
 * so the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss02816_800C1B40(void) {
  D_801463A4 = func_800C1B80;
  D_801463A8 = invokeZeroArgBossBoss02816_800C1BCC;
  D_801463AC = (Boss02816Handler)zeroResultBossBoss02816_800C1B78;
}
