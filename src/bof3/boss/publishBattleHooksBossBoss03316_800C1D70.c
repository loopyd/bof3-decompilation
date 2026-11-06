#include "bof3/bof3.h"

typedef void (*Boss03316Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch and the second is an inert handler; both are void routines. The
 * third returns a zero result, which the void slot type drops. */
void func_800C1DB0(void);
void noopHandlerBossBoss03316_800C1E24(void);
s32 zeroResultBossBoss03316_800C1DA8(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03316Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03316Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03316Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1D70
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1DB0 (state switch: tests the 0x6 bits of D_80146328 and, while any
 * is set, calls func_800AC934 for channel 0, publishes D_80145EC4/D_80145EC8
 * into 0x80149308/0x8014930C, stores 8 into D_80146875 and calls func_801DDED8,
 * and otherwise calls func_801DDF00) into 0x801463A4, the inert
 * noopHandlerBossBoss03316_800C1E24 into 0x801463A8, and the zero-result
 * accessor zeroResultBossBoss03316_800C1DA8 into 0x801463AC; the accessor's s32
 * result is dropped by the void slot type. Same trio shape as the sibling
 * installers of this archive family, which write the same three slots in the
 * same order. No in-payload caller: neither this function's address nor the
 * three slot addresses appear in any other word of this payload, so the main
 * program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss03316_800C1D70(void) {
  D_801463A4 = func_800C1DB0;
  D_801463A8 = noopHandlerBossBoss03316_800C1E24;
  D_801463AC = (Boss03316Handler)zeroResultBossBoss03316_800C1DA8;
}
