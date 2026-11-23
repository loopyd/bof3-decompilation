#include "bof3/bof3.h"

u8* findFreeWorkRecordScenarioScena0700_801F8394(void);
void func_801F8478(u8* record);
s32 func_801F85D8(void);
void func_801F9330(u8* p);
extern u8* scenarioScena0700Work; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F7AB0
 * @behavior Entry 1, the middle entry of this overlay's three-entry work-state
 * handler run at 0x801FDDB4 (0x801F7A30 / this function /
 * clearWorkFlagsWhenNoCallbackRanScenarioScena0700_801F7B68), which the
 * dispatcher dispatchWorkByte2HandlerScenarioScena0700_801F79EC indexes with
 * the work-object byte at scratchpad pointer slot 0x1F800044 + 0x02.
 * It first hands the pointer read from the overlay cell 0x801FDFC0 - the cell
 * func_801F78B0 stores the shared 0x800E4C24 block's address into - to the
 * overlay's pointer-taking worker at 0x801F9330. It then reads the scratchpad
 * work object: while that object's 32-bit word at offset 0x0C still reads at
 * least 9 it claims one work record through the 0x8-entry, 0x1C-byte-stride
 * scan at 0x801F8394, and only on the frames whose word at 0x0C reads zero in
 * its low two bits, passing a non-zero record in $a0 to the record seeder at
 * 0x801F8478. The two tests share one exit label, so a frame outside the window
 * or off the four-frame phase falls through to the step below.
 * It then calls the work-table stepper at 0x801F85D8 unconditionally, re-reads
 * the scratchpad work-object pointer (the call clobbers the temporary
 * registers, so the cell is loaded again into $v1), decrements the object's
 * 32-bit word at 0x0C in place - the `sw` of the decremented value lands in the
 * branch delay slot - and, only when the decremented count reads zero,
 * increments the object's byte at 0x02 by one so the dispatcher moves the run
 * on to its next entry.
 * The word at 0x0C is the run's countdown: entry 0 of the same run,
 * func_801F7960, tests that word against the same bound of 9, decrements it in
 * the same place and acts on the zero result the same way, so both entries walk
 * one shared counter.
 * It takes no arguments, returns nothing and its 0x18-byte frame keeps only
 * $ra, whose call delay slots stay nops (nothing is live into them).
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimEveryFourthWhile(
    void) {
  u8* record;
  u8* work;

  func_801F9330(*PSX_PTR(u8*, 0x801FDFC0u));

  if (*(s32*)(scenarioScena0700Work + 0xC) >= 9) {
    record = findFreeWorkRecordScenarioScena0700_801F8394();
    if ((*(s32*)(scenarioScena0700Work + 0xC) & 3) == 0 && record != 0) {
      func_801F8478(record);
    }
  }

  func_801F85D8();

  work = scenarioScena0700Work;

  if (--*(s32*)(work + 0xC) == 0) {
    work[2]++;
  }
}
