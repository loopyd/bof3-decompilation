#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

void applyMainLookupRecord30CClearWorkByte48Byte24AndWord3CCallPairEntry1ThenAdvanceWorkByte2BossBoss05516_800C28E0(
    void);
void copyTemplateRecordThenRequestByteAtIndexedByBattleHalfwordWhenGlobalWorkByteBSetAndRefreshWorkBossBoss05516_800C2948(
    void);

/* @source 0x800C2888
 * @behavior Overlay step dispatch on the state byte of the work record
 * published at scratchpad pointer cell 0x1F800044: seeds a local two-entry
 * handler array with 0x800C28E0 then 0x800C2948 and calls the entry selected by
 * the record's unsigned byte +2 without arguments. The original reads the
 * work-record pointer once, fills both frame slots ahead of the byte load and
 * does not bound the index, and it writes no state of its own: the 0x20-byte
 * frame only keeps $ra across the indirect call. Its body is the same 17
 * instructions as 0x800C2728 and 0x800C2B48 and differs from them only in the
 * pair it seeds. The word at 0x800C35FC holds this function's address, so it is
 * entry 1 of the 0x800C35F8 step-handler run that the state byte +1 of the same
 * work record indexes; entry 0 (0x800C2728) selects the step it precedes. Of
 * this pair, 0x800C28E0 performs setup and advances byte +2 by one, so it hands
 * that record to the 0x800C2948 step body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2LocalPairBossBoss05516_800C2888(void) {
  Boss05516Handler handlers[2] = {
      applyMainLookupRecord30CClearWorkByte48Byte24AndWord3CCallPairEntry1ThenAdvanceWorkByte2BossBoss05516_800C28E0,
      copyTemplateRecordThenRequestByteAtIndexedByBattleHalfwordWhenGlobalWorkByteBSetAndRefreshWorkBossBoss05516_800C2948};

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
