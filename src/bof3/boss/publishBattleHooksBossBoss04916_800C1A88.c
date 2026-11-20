#include "bof3/bof3.h"

typedef void (*Boss04916Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * late-phase transition handler: it loads the battle flag byte D_80146328, and
 * when its 0x2 bit is set stores 10 into D_80146875 and calls func_801DDED8,
 * and otherwise calls func_801DDF00. The second is an inert handler of this
 * overlay. The third returns a zero result, which the void slot type drops. */
void func_800C1AC8(void);
void noopHandlerBossBoss04916_800C1B14(void);
s32  zeroResultBossBoss04916_800C1AC0(void);

/* Shared hook slots, the same trio the earlier installer of this overlay at
 * 0x800C19F4 writes. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss04916Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss04916Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss04916Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1A88
 * @behavior Publishes this overlay's late-phase handler trio into the shared
 * hook slots: func_800C1AC8 (mode-transition handler: tests the 0x2 bit of
 * D_80146328 and, when it is set, stores 10 into D_80146875 and calls
 * func_801DDED8, and otherwise calls func_801DDF00) into 0x801463A4, the inert
 * handler noopHandlerBossBoss04916_800C1B14 into 0x801463A8, and the
 * zero-result accessor zeroResultBossBoss04916_800C1AC0 into 0x801463AC; the
 * accessor's s32 result is dropped by the void slot type. Each value is
 * materialized with a lui/addiu address pair and stored through a lui base, so
 * the body returns without reading any state of its own. Same trio shape as
 * the earlier installer of this overlay at 0x800C19F4, which writes the same
 * three slots in the same order with func_800C1A34,
 * noopHandlerBossBoss04916_800C1A80 and zeroResultBossBoss04916_800C1A2C. No
 * in-payload caller: this function's address appears in no other word of this
 * payload and no jal reaches it, so the main program reaches it through its
 * overlay entry point. The 0x38 bytes of this span at payload offset 0x288
 * compare equal to the span at the same offset of BIN/BOSS/BOSS040.EMI#16, a
 * payload with the same 0x800C1800 load address whose three published
 * entrypoints and three hook slots sit at the same addresses as here, so the
 * body transfers unchanged from its exact lift
 * publishBattleHooksBossBoss04016_800C1A88.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss04916_800C1A88(void) {
  D_801463A4 = func_800C1AC8;
  D_801463A8 = noopHandlerBossBoss04916_800C1B14;
  D_801463AC = (Boss04916Handler)zeroResultBossBoss04916_800C1AC0;
}
