#include "bof3/bof3.h"

typedef void (*Boss01416Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it reads the shared battle flag byte 0x80146328 and tests bit
 * 0x2, and while that bit is set it stores 0xC into the shared scenario progress
 * byte 0x80146864 and runs initModeTuple510 (0x801DDED8), and otherwise runs
 * initModeTuple520 (0x801DDF00); it keeps the return address at 0x10 of a
 * 0x18-byte frame across the chosen call. The second is a thin wrapper that
 * calls the helper at 0x800AC934 with argument zero. The third returns a zero
 * result, which the void slot type drops. */
void func_800C1A28(void);
void invokeZeroArgBossBoss01416_800C1A74(void);
s32  zeroResultBossBoss01416_800C1A20(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss01416Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss01416Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss01416Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C19E8
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1A28 (state switch: tests bit 0x2 of the shared battle flag byte
 * 0x80146328 and, while it is set, stores 0xC into the shared scenario progress
 * byte 0x80146864 and runs initModeTuple510, and otherwise runs
 * initModeTuple520) into 0x801463A4, the thin wrapper
 * invokeZeroArgBossBoss01416_800C1A74 (calls the helper at 0x800AC934 with
 * argument zero) into 0x801463A8, and the zero-result accessor
 * zeroResultBossBoss01416_800C1A20 into 0x801463AC; the accessor's s32 result is
 * dropped by the void slot type. Each slot is built as an address pair and
 * stored through $at, so the body keeps no frame, touches no memory but those
 * three words and returns through $ra. No word of this payload holds this
 * function's address as an operand and no call reaches it, so the main program
 * reaches it through its overlay entry point. Same trio shape as the sibling
 * installers of this archive family, which write the same three slots in the
 * same order; a payload comparison showed boss024's installer at this same
 * overlay offset 0x800C19E8 holds the same fourteen words except the low half of
 * one address constant, and that sibling was lifted exact from its own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss01416_800C19E8(void) {
  D_801463A4 = func_800C1A28;
  D_801463A8 = invokeZeroArgBossBoss01416_800C1A74;
  D_801463AC = (Boss01416Handler)zeroResultBossBoss01416_800C1A20;
}
