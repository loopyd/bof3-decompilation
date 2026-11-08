#include "bof3/bof3.h"

extern int rand(void);

typedef void (*Boss01316Handler)(void);

/* Overlay handlers this installer publishes. The first two take no arguments
 * and return nothing; the third ends with a zero result, which the void slot
 * type drops. */
void initModeTuple510Or530ByFlagBitTwoSetScenarioProgress14Or50StoreHalfwordSumApplyRecordF9ThenClearHalfwordBit80BossBoss01316_800C2348(void);
void func_800C243C(void);
s32  func_800C2138(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle step
 * loads and calls (jalr); 0x801463AC is the third slot of the same trio, written
 * by every boss overlay of this archive family. */
extern Boss01316Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss01316Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss01316Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* Step countdown byte of this payload's own data run: the last word before the
 * run end at 0x800C26E4, zero in the payload. The dispatched handler
 * func_800C1E90 tests it, stores one less, and takes its fire path when it is
 * zero; the published hook func_800C2138 tests it as a guard before its work
 * and clears it. */
extern u8 D_800C26E0; /* @source 0x800C26E0 @kind data */

/* @source 0x800C20D8
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * initModeTuple510Or530ByFlagBitTwoSetScenarioProgress14Or50StoreHalfwordSumApplyRecordF9ThenClearHalfwordBit80BossBoss01316_800C2348
 * (0x800C2348) into 0x801463A4, func_800C243C into 0x801463A8 and
 * func_800C2138 into 0x801463AC, then seeds the overlay step countdown byte at
 * 0x800C26E0 with 8 plus the low two bits of rand() (andi 0x2, addiu 8), so the
 * countdown starts at 8, 9 or 10. The 0x18-byte frame exists only to keep $ra
 * across the rand() call; the three slot stores carry their own
 * lui $at/%hi pair, and the byte store materialises its own base the same way.
 * Same trio shape as the sibling installer
 * publishBattleHooksBossBoss01316_800C200C of this target, which writes the
 * same three slots in the same order and differs in the published addresses and
 * in having no countdown seed. No in-payload caller: no word of this payload
 * holds 0x800C20D8 and no jal reaches it, so the main program reaches it
 * through its overlay entry point, and the countdown it seeds is the field the
 * dispatched handler func_800C1E90 decrements.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksAndSeedStepCountdownBossBoss01316_800C20D8(void) {
  D_801463A4 = initModeTuple510Or530ByFlagBitTwoSetScenarioProgress14Or50StoreHalfwordSumApplyRecordF9ThenClearHalfwordBit80BossBoss01316_800C2348;
  D_801463A8 = func_800C243C;
  D_801463AC = (Boss01316Handler)func_800C2138;
  D_800C26E0 = (rand() & 2) + 8;
}
