#include "bof3/bof3.h"

/* Shared main-RAM byte this overlay's previous work step (0x801F71DC) stores 2
 * into just before it sets the work object's byte 0x01 to 1; this handler reads
 * it with `lbu` and re-arms the work object only while it is not yet 2. */
extern u8 D_80143BB0;

void func_801F7484(void);
void setWorkByte29To2OnByte6IndexedRecordAndBaseRecordThenInvokeSharedServiceScenarioScena1300_801F7568(
    void);

/* @source 0x801F7248
 * @behavior Work-object step handler of this overlay, the word at 0x801FBF00:
 * entry 1 of the sub-run D_801FBEFC that the frame dispatcher func_801F7198
 * indexes with the work object's byte 0x01, and entry 4 of the twelve-word
 * handler table D_801FBEF0 that func_801F6EA4 indexes with the same byte. It
 * takes no arguments and returns nothing. It first reads the shared main-RAM
 * byte D_80143BB0 with `lbu` (`lui $v1,0x8014` / `lbu $v1,0x3BB0($v1)`) and
 * compares it against 2 (`addiu $v0,$zero,0x2` / `beq $v1,$v0`); only while that
 * byte is not 2 does it re-read the work-object pointer cell 0x1F800044 (`lui
 * $v1,0x1F80` / `lw $v1,0x44($v1)`), store the countdown 0x1E into the object's
 * byte 0x09 (`addiu $v0,$zero,0x1E` / `sb $v0,0x9($v1)`), read that same cell a
 * second time the same way and store 2 into the object's byte 0x01.
 * Both arms rejoin at the two overlay helper calls func_801F7484 and
 * setWorkByte29To2OnByte6IndexedRecordAndBaseRecordThenInvokeSharedServiceScenarioScena1300_801F7568,
 * made in that order with no arguments and with any result
 * discarded, so the pair runs on every call; their jal delay slots stay nops.
 * The 0x18-byte frame keeps only $ra at 0x10 across them, and $ra is stored in
 * the delay slot of the shared-byte compare, so the frame carries no other
 * state. No word in this payload encodes a jal to 0x801F7248, so the handler
 * tables are the only references to it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armWorkCountdownAndAdvanceWorkByte1WhileSharedByteNotTwoThenInvokeTwoHelpersScenarioScena1300_801F7248(void) {
  u32 slot_offset;
  u8 *work;

  if (D_80143BB0 != 2) {
    slot_offset = 0x44u;
    work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    work[9] = 0x1E;
    PSX_REF(u8 *, SPAD_BASE + slot_offset)[1] = 2;
  }
  func_801F7484();
  setWorkByte29To2OnByte6IndexedRecordAndBaseRecordThenInvokeSharedServiceScenarioScena1300_801F7568();
}
