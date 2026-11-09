#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

void func_800C2A40(void);
void copyTemplateRecordThenRequestByteAtIndexedByBattleHalfwordWhenGlobalWorkByteBSetAndRefreshWorkBossBoss05516_800C2AA8(
    void);

/* @source 0x800C29E8
 * @behavior Overlay step dispatch on the state byte of the work record
 * published at scratchpad pointer cell 0x1F800044: seeds a local two-entry
 * handler array with 0x800C2A40 then 0x800C2AA8 and calls the entry selected by
 * the record's unsigned byte +2 without arguments. The original reads the
 * work-record pointer once, fills both frame slots ahead of the byte load and
 * does not bound the index, and it writes no state of its own: the 0x20-byte
 * frame only keeps $ra across the indirect call. Its body is the same 22
 * instructions and 0x58 bytes as the exact siblings 0x800C2728 and 0x800C2888
 * and differs from them only in the pair it seeds. The word at 0x800C3600 holds
 * this function's address, so it is entry 2 of the 0x800C35F8 step-handler run
 * this overlay's state byte +1 indexes, and word 11 of the 0x800C35D4 pointer
 * run that holds the run. Entry 0 of the seeded pair (0x800C2A40) performs
 * setup, calls 0x800C2AA8 and then advances byte +2 of the same record by one,
 * so the pair is this overlay's next two step bodies, mirroring the
 * 0x800C28E0/0x800C2948 pair the preceding run entry dispatches.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2LocalPairBossBoss05516_800C29E8(void) {
  Boss05516Handler handlers[2] = {
      func_800C2A40,
      copyTemplateRecordThenRequestByteAtIndexedByBattleHalfwordWhenGlobalWorkByteBSetAndRefreshWorkBossBoss05516_800C2AA8};

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
