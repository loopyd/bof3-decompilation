#include "bof3/bof3.h"

u8* findFreeWorkRecordScenarioScena0700_801F8394(void);
void func_801F83D0(u8* record);
s32 func_801F85D8(void);

/* @source 0x801F6E90
 * @behavior Overlay work-state handler for scratch work-object byte 2 = 1, the
 * middle entry of the three-entry handler run at 0x801FDD3C: it reads the
 * scratchpad work object published at pointer cell 0x1F800044 (the read is
 * scheduled ahead of the frame reservation, so its `lui`/`lw` pair is the
 * function's first two instructions) and, on the frames whose 32-bit word at
 * offset 0x0C reads zero in its low three bits, claims one work record through
 * the table scan at 0x801F8394 and, when that scan returns a non-zero record,
 * passes it in $a0 to the record seeder at 0x801F83D0. Both calls sit inside the
 * same guard and share its exit label, so a frame with no free record falls
 * through to the next step. It then calls the work-table stepper at 0x801F85D8
 * unconditionally, re-reads the scratchpad work-object pointer (the call
 * clobbers the registers, so the cell is loaded again into $v1), decrements the
 * object's 32-bit word at 0x0C in place - the `sw` of the decremented value
 * lands in the branch delay slot - and, only when the decremented count reads
 * zero, increments the object's byte at 0x02 by one so the dispatcher at
 * 0x801F6E10 moves the run on to its next entry.
 * The two written fields are the ones this run shares: the entry-0 handler
 * stateTableIncrementCountdown stores
 * 0x40 into word 0x0C and increments byte 0x02, and that word is the countdown
 * this body tests and decrements, so 0x40 frames give the masked claim eight
 * times (0x40, 0x38 ... 0x08) before the count reaches zero and byte 0x02
 * advances. It takes no arguments, returns nothing, and its 0x18-byte frame
 * keeps only $ra, whose call delay slots stay nops (nothing is live into them).
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeEveryEighth(
    void) {
  u8* record;
  u8* work;

  if ((*(s32*)(SPAD_PTR_SLOT(u8, 0x44u) + 0xC) & 7) == 0) {
    record = findFreeWorkRecordScenarioScena0700_801F8394();
    if (record != 0) {
      func_801F83D0(record);
    }
  }

  func_801F85D8();

  work = SPAD_PTR_SLOT(u8, 0x44u);

  if (--*(s32*)(work + 0xC) == 0) {
    work[2]++;
  }
}
