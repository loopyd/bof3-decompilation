#include "bof3/bof3.h"

typedef void (*Boss00216Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the
 * mode-transition handler initModeTuple510Or520ByFlagBitTwoBossBoss00216_800C1F48,
 * which tests the 0x2 mask of D_80146328 and either stores 0x5 into D_80146875
 * and calls func_801DDED8, or calls func_801DDF00.
 * The second takes no argument and touches no state. The third returns a zero
 * result, which the void slot type drops. */
void initModeTuple510Or520ByFlagBitTwoBossBoss00216_800C1F48(void);
void noopHandlerBossBoss00216_800C1F94(void);
s32 zeroResultBossBoss00216_800C1F40(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss00216Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss00216Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss00216Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1F08
 * @behavior Second hook installer of this overlay: publishes a handler trio
 * into the shared slots in slot order, the mode switch func_800C1F48 (tests
 * D_80146328 bit 0x2, writes scenario progress 0x5 or forwards to the
 * complementary routine) into 0x801463A4, the empty handler
 * noopHandlerBossBoss00216_800C1F94 into 0x801463A8, and the zero-result
 * accessor zeroResultBossBoss00216_800C1F40 into 0x801463AC through the void
 * slot type. Same shape as the sibling installer at 0x800C1E3C of this
 * overlay, which fills the trio with the progress/argument/accessor variants.
 * No payload word names this function and no jal reaches it, so the main
 * program enters it through the overlay entry point, as it does the sibling
 * installers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss00216_800C1F08(void) {
  D_801463A4 = initModeTuple510Or520ByFlagBitTwoBossBoss00216_800C1F48;
  D_801463A8 = noopHandlerBossBoss00216_800C1F94;
  D_801463AC = (Boss00216Handler)zeroResultBossBoss00216_800C1F40;
}
