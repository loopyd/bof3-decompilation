#include "bof3/bof3.h"

typedef void (*Boss03616Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 5 into the byte at D_80146875 and calls func_801DDED8, and otherwise calls
 * func_801DDF00. The second is an inert handler that returns immediately. The
 * third returns a zero result, which the void slot type drops. */
void func_800C1E4C(void);
void noopHandlerBossBoss03616_800C1E98(void);
s32  zeroResultBossBoss03616_800C1E44(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss03616Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03616Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03616Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1E0C
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1E4C (state switch: tests the 0x2 bit of D_80146328 and, while it is
 * set, stores 5 into the byte at D_80146875 and calls func_801DDED8, and
 * otherwise calls func_801DDF00) into 0x801463A4, the inert handler
 * noopHandlerBossBoss03616_800C1E98 into 0x801463A8, and the zero-result
 * accessor zeroResultBossBoss03616_800C1E44 into 0x801463AC; the accessor's s32
 * result is dropped by the void slot type. Same trio shape as the sibling
 * installers of this archive family, which write the same three slots in the
 * same order and differ only in the published addresses. No in-payload caller:
 * neither this function's address nor the three slot addresses appear in any
 * other word of this payload and no jal reaches it, so the main program reaches
 * it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss03616_800C1E0C(void) {
  D_801463A4 = func_800C1E4C;
  D_801463A8 = noopHandlerBossBoss03616_800C1E98;
  D_801463AC = (Boss03616Handler)zeroResultBossBoss03616_800C1E44;
}
