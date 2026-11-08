#include "bof3/bof3.h"

typedef void (*Boss00716StepHandler)(u8 index);

/* This payload's data row at 0x800C1D78, the 160 bytes the Splat `textbin` row
 * places at the end of the segment: 0x800C1D84 is that row's +0xC byte, where
 * its ten-byte 0x01 run starts, and 0x800C1D98 is its +0x20 byte inside the
 * 0xFF run that reaches 0x800C1DA0, the row's step-handler table. Neither is a
 * row boundary, so each is referenced as its own data symbol rather than as a
 * row start. */
extern u8 D_800C1D84; /* @source 0x800C1D84 @kind data */
extern u8 D_800C1D98; /* @source 0x800C1D98 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratchpad work-object pointer cell. The original re-materialises this cell
 * before each of its two accesses, `lui` 0x1F80 plus `lw 0x44` at 0x15C/0x160
 * and again at 0x190/0x194, so the source names the cell and evaluates it at
 * every access instead of letting the bare constant fold into one `lui`/`ori`
 * pair.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* This payload's next step dispatcher at 0x800C19B4: it masks its argument
 * with 0xFF, scales it by four and calls the entry of the 0x800C1E0C run, the
 * 0x94-byte offset inside the same 0x800C1D78 row. */
void func_800C19B4(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1950
 * @behavior Step setup of this overlay's work state machine, entry 0 of the
 * twelve-word handler run based at 0x800C1DDC: publishes three words into the
 * current enemy work record loaded from 0x801EB4E8 -- the data row pointer
 * 0x800C1D84 (row +0xC) into +0xEC, the step dispatcher func_800C19B4 into
 * +0xE4 and the in-row pointer 0x800C1D98 (row +0x20) into +0xE8 -- then
 * clears byte +0x8 of the work record the main program publishes at the
 * scratchpad pointer cell 0x1F800044, stores 2 into that record's byte +1 and
 * calls func_8014D978. The original loads that pointer cell twice, once into
 * $a0 for the byte-+0x8 clear and once into $v1 for the state byte across the
 * three record stores, so each access keeps its own load; the first one is
 * held in a pointer local because a store through the named cell cannot be
 * moved behind the three record stores, while the store it feeds sits after
 * them. Byte +1 is the value dispatchStateByte1Table1ddcBossBoss00716_800C190C
 * indexes its 0x800C1DDC run by, so writing 2 hands the next step to that run's
 * entry 2; the role of byte +0x8 is not established here. No jal in the payload
 * names 0x800C1950: its only in-image reference is the word 0x800C1DDC of that
 * same data row. The three installs, the state byte and the tail call are the
 * already-exact publishers
 * publishWorkStepHandlerAndDataThenSetStateTo2BossBoss00716_800C1864 of this
 * payload and publishWorkStepHandlerAndDataThenSetStateTo2BossBoss03016_800C1848,
 * which differ only in the immediates of what they install; the byte-+0x8 clear
 * is the one statement only this body has. Both prologue and return path place
 * the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenClearWorkByte8AndSetStateTo2BossBoss00716_800C1950(
    void) {
  u8 *work = D_801EB4E8;
  u8 *record;

  record = D_1F800044;
  FIELD_REF(void *, work, 0xECu) = &D_800C1D84;
  FIELD_REF(Boss00716StepHandler, work, 0xE4u) = func_800C19B4;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1D98;
  record[8] = 0;
  D_1F800044[1] = 2;
  func_8014D978();
}
