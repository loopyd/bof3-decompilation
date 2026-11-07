#include "bof3/bof3.h"

typedef void (*Boss01416Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 0x5 into the byte at 0x80146875 and calls func_801DDED8, and otherwise calls
 * func_801DDF00. The second is an inert handler; the third returns a zero
 * result, which the void slot type drops. */
void func_800C1AD4(void);
void noopHandlerBossBoss01416_800C1B20(void);
s32  zeroResultBossBoss01416_800C1ACC(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss01416Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss01416Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss01416Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1A94
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1AD4 (state switch: tests the 0x2 bit of D_80146328 and, while it
 * is set, stores 0x5 into the byte at 0x80146875 and calls func_801DDED8, and
 * otherwise calls func_801DDF00) into 0x801463A4, the inert
 * noopHandlerBossBoss01416_800C1B20 into 0x801463A8, and the zero-result
 * accessor zeroResultBossBoss01416_800C1ACC into 0x801463AC; the accessor's s32
 * result is dropped by the void slot type. Same trio shape as the sibling
 * installers of this archive family, which write the same three slots in the
 * same order. No in-payload caller: this function's address appears in no other
 * word of this payload and no jal reaches it, so the main program reaches it
 * through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss01416_800C1A94(void) {
  D_801463A4 = func_800C1AD4;
  D_801463A8 = noopHandlerBossBoss01416_800C1B20;
  D_801463AC = (Boss01416Handler)zeroResultBossBoss01416_800C1ACC;
}
