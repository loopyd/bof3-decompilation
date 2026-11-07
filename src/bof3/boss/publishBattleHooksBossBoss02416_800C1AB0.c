#include "bof3/bof3.h"

typedef void (*Boss02416Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, calls
 * func_801DDED8, and otherwise calls func_801DDF00. The second is an inert
 * handler that returns immediately, and the third returns a zero result, which
 * the void slot type drops. */
void func_800C1AF0(void);
void noopHandlerBossBoss02416_800C1B3C(void);
s32  zeroResultBossBoss02416_800C1AE8(void);

/* Shared hook slots, the same trio the sibling installer at 0x800C19E8 writes:
 * 0x801463A4/0x801463A8 are the battle hooks the battle step loads and calls
 * (jalr); 0x801463AC is the third slot of the trio, written by every boss
 * overlay of this archive family. */
extern Boss02416Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02416Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02416Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1AB0
 * @behavior Publishes a second handler trio into the shared hook slots:
 * func_800C1AF0 (the state switch: tests the 0x2 bit of D_80146328 and, while
 * it is set, calls func_801DDED8, and otherwise calls func_801DDF00) into
 * 0x801463A4, the inert handler noopHandlerBossBoss02416_800C1B3C into
 * 0x801463A8, and the zero-result accessor
 * zeroResultBossBoss02416_800C1AE8 into 0x801463AC; the accessor's s32 result
 * is dropped by the void slot type. Same trio shape as the sibling installer at
 * 0x800C19E8, which writes the same three slots in the same order; the two
 * bodies differ only in the three stored pointers. No in-payload caller:
 * neither this function's address appears in any other word of this payload nor
 * does any jal reach the overlay's code, so the main program reaches it through
 * its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss02416_800C1AB0(void) {
  D_801463A4 = func_800C1AF0;
  D_801463A8 = noopHandlerBossBoss02416_800C1B3C;
  D_801463AC = (Boss02416Handler)zeroResultBossBoss02416_800C1AE8;
}
