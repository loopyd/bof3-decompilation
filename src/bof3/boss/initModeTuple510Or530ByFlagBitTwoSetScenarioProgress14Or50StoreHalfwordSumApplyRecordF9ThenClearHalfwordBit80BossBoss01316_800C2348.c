#include "bof3/bof3.h"

extern u8 D_80145029;         /* @source 0x80145029 @kind unknown */
extern u16 D_80146258;        /* @source 0x80146258 @kind unknown */
extern u8 D_801462ED;         /* @source 0x801462ED @kind unknown */
extern u8 D_80146328;         /* @source 0x80146328 @kind unknown */
extern u32 D_8014632C;        /* @source 0x8014632C @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */
extern u16 D_801EB6B6;        /* @source 0x801EB6B6 @kind unknown */
extern u16 D_801EB7CE;        /* @source 0x801EB7CE @kind unknown */
extern u8 D_801EB860;         /* @source 0x801EB860 @kind unknown */

/* Scratchpad work-record pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. The original
 * re-reads the cell after the first call because the store through the loaded
 * pointer may alias the cell, so each access names the symbol instead of
 * keeping a pointer in a local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program mode-tuple services this archive family runs at the end of
 * its battle-flag state switch: 0x801DDED8 seeds the three-byte battle mode
 * tuple with 5, 1 and 0, and 0x801DDF28 seeds it with 5, 3 and 0. */
void initModeTuple510(void); /* @source 0x801DDED8 */
void initModeTuple530(void); /* @source 0x801DDF28 */

/* Shared main-executable lookup helper. Its exact lift at 0x8014DD3C shows it
 * applying the "main lookup" record whose id matches its argument to the active
 * work record, so the argument here is a record id and the 0xF9 request names
 * that record. This target's generated undefined_funcs_auto row binds the
 * address-anchored name, so the call keeps it. */
void func_8014DD3C(s32 entry); /* @source 0x8014DD3C */

/* Resource request helper of the main executable, reached through the same
 * generated undefined_funcs_auto row; the sibling boss and bmagic lifts declare
 * the same prototype and pass a resource id. */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* @source 0x800C2348
 * @behavior Battle-hook installer step of this overlay and first member of the
 * handler trio that publishBattleHooksAndSeedStepCountdownBossBoss01316_800C20D8
 * (0x800C20D8) materializes into the shared battle hook slot 0x801463A4: that
 * installer stores this address with lui/addiu, so no jal in the payload reaches
 * this body. It loads the shared battle flag byte 0x80146328 once and tests bit
 * 0x2. While the bit is set it stores 0x14 into the shared scenario progress
 * byte 0x80146864, stores the sum of the two shared halfwords 0x801EB6B6 and
 * 0x801EB7CE into the shared word 0x8014632C, and runs initModeTuple510
 * (0x801DDED8), which seeds the three-byte battle mode tuple with 5, 1 and 0.
 * While the bit is clear it tests bit 0x40 of the shared flag byte 0x801462ED:
 * when that bit is set it publishes the shared record 0x801EB860 into the
 * scratchpad pointer cell 0x1F800044, applies main lookup record 0xF9 through
 * the shared helper 0x8014DD3C, clears bit 0 of that record's byte at offset
 * 0x24, stores 1 into that record's byte at offset 0x2A, and requests resource 1
 * through the shared helper 0x8014D6B8. Both clear-path outcomes then store
 * 0x50 into 0x80146864 and run initModeTuple530 (0x801DDF28), which seeds the
 * mode tuple with 5, 3 and 0. The shared tail that both paths reach stores 0x31
 * into the shared byte 0x80145029 and clears bit 0x0080 of the shared halfword
 * 0x80146258 with the 0xFF7F mask (andi 0xFF7F after lhu). It takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps the return
 * address at 0x10 across the chosen mode-tuple call, the 0x50 constant load
 * fills the delay slot of the nested test, and the slots after the flag byte
 * load, after the cell load and after the halfword load stay nops. Same flag
 * byte, bit and mode-tuple pair as the exact sibling initModeTuple510Or530ByFlagBitTwoSetScenarioProgress32AndModeByte3BossBoss01316_800C204C
 * one stage earlier in this payload; this body differs in the scenario progress
 * value it stores on both paths, the halfword sum it stores on the set path, the
 * nested 0x801462ED test and the record setup it runs on the clear path, and the
 * shared tail it ends with.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initModeTuple510Or530ByFlagBitTwoSetScenarioProgress14Or50StoreHalfwordSumApplyRecordF9ThenClearHalfwordBit80BossBoss01316_800C2348(
    void) {
  if (D_80146328 & 0x2) {
    g_ScenarioProgress = 0x14;
    D_8014632C = D_801EB6B6 + D_801EB7CE;
    initModeTuple510();
  } else {
    if (D_801462ED & 0x40) {
      D_1F800044 = &D_801EB860;
      func_8014DD3C(0xF9);
      D_1F800044[0x24] &= 0xFE;
      D_1F800044[0x2A] = 0x1;
      func_8014D6B8(1);
    }
    g_ScenarioProgress = 0x50;
    initModeTuple530();
  }
  D_80145029 = 0x31;
  D_80146258 &= 0xFF7F;
}
