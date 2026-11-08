#include "bof3/bof3.h"

typedef void (*Boss01316Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the overlay's
 * state switch: it tests the 0x2 bit of D_80146328, stores 0x32 into the byte
 * at g_ScenarioProgress and then calls func_801DDED8 while the bit is set, and
 * otherwise stores 0x32 there, calls func_801DDF28 and writes 0x3 into the byte
 * at 0x80146255. The second clears the 0x40 flag of the battle records owned by
 * id 0 and id 1. The third returns a zero result, which the void slot type
 * drops. */
void initModeTuple510Or530ByFlagBitTwoSetScenarioProgress32AndModeByte3BossBoss01316_800C204C(void);
void clearOwnerRecordFlag40OnOwnerZeroThenOwnerOneBossBoss01316_800C20B0(void);
s32  zeroResultBossBoss01316_800C2044(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss01316Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss01316Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss01316Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C200C
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * the state switch initModeTuple510Or530ByFlagBitTwoSetScenarioProgress32AndModeByte3BossBoss01316_800C204C
 * (tests the 0x2 bit of D_80146328 and stores 0x32 into the byte at
 * g_ScenarioProgress, then calls func_801DDED8 while the bit is set and
 * otherwise calls func_801DDF28 and writes 0x3 into the byte at 0x80146255)
 * into 0x801463A4, the owner-record flag clearer
 * clearOwnerRecordFlag40OnOwnerZeroThenOwnerOneBossBoss01316_800C20B0 into
 * 0x801463A8, and the zero-result accessor
 * zeroResultBossBoss01316_800C2044 into 0x801463AC; the accessor's s32 result is
 * dropped by the void slot type. Same trio shape as the sibling installers of
 * this archive family, which write the same three slots in the same order and
 * differ only in the published addresses. No in-payload caller: this function's
 * address appears in no other word of this payload and no jal reaches it, so
 * the main program reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss01316_800C200C(void) {
  D_801463A4 =
      initModeTuple510Or530ByFlagBitTwoSetScenarioProgress32AndModeByte3BossBoss01316_800C204C;
  D_801463A8 = clearOwnerRecordFlag40OnOwnerZeroThenOwnerOneBossBoss01316_800C20B0;
  D_801463AC = (Boss01316Handler)zeroResultBossBoss01316_800C2044;
}
