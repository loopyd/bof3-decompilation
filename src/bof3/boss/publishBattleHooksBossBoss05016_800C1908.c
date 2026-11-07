#include "bof3/bof3.h"

typedef void (*Boss05016Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328 and, while it is set, stores
 * 0x10 into D_80146875, calls func_801DDED8 and then stores 0x8C into
 * D_80145029, and otherwise calls func_801DDF00. The second is an inert
 * handler; the third returns a zero result, which the void slot type drops. */
void func_800C1948(void);
void noopHandlerBossBoss05016_800C19A0(void);
s32  zeroResultBossBoss05016_800C1940(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss05016Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss05016Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss05016Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1908
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * func_800C1948 (state switch: tests the 0x2 bit of D_80146328 and, while it is
 * set, stores 0x10 into D_80146875, calls func_801DDED8 and then stores 0x8C
 * into D_80145029, and otherwise calls func_801DDF00) into 0x801463A4, the
 * inert noopHandlerBossBoss05016_800C19A0 into 0x801463A8, and the zero-result
 * accessor zeroResultBossBoss05016_800C1940 into 0x801463AC; the accessor's s32
 * result is dropped by the void slot type. Same trio shape as the two exact
 * sibling installers of this archive family, at 0x800C1930 of
 * BIN/BOSS/BOSS054.EMI#16 and 0x800C1B0C of BIN/BOSS/BOSS052.EMI#16, which
 * write the same three slots in the same order; the family has no
 * byte-identical twin to clone here because each member encodes its own
 * published handler addresses in the instruction immediates. No in-payload
 * caller: neither this function's address nor the three slot addresses appear
 * in any other word of this payload, and no jal reaches it, so the main program
 * reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss05016_800C1908(void) {
  D_801463A4 = func_800C1948;
  D_801463A8 = noopHandlerBossBoss05016_800C19A0;
  D_801463AC = (Boss05016Handler)zeroResultBossBoss05016_800C1940;
}
