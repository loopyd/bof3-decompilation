#include "bof3/bof3.h"

void func_801F6FF8(void);
void setWorkByte29To5OnActiveWorkRecordsWithByte6SixScenarioScena1300_801F70DC(
    void);

/* @source 0x801F6F24
 * @behavior Entry 1 of this overlay's twelve-word work-object handler table
 * D_801FBEF0 (its word at 0x801FBEF4), the entry the byte-1 dispatcher
 * func_801F6EA4 selects once the previous step
 * clearWorkTintArmCountdownAndAdvanceWorkByte1ScenarioScena1300_801F6EE8 has
 * zeroed that object's tint triplet, armed its countdown byte 0x09 with 0xF and
 * moved its dispatch byte 0x01 to 1. It takes no arguments and returns nothing;
 * it reaches the work object only through the scratchpad pointer cell
 * 0x1F800044. The first group reads the cell once (`lui $v0,0x1F80` / `lw
 * $v0,0x44($v0)`), takes the object's byte 0x09 (`lbu $v1,0x9($v0)`), subtracts
 * one (`addiu $v1,$v1,-0x1`) and stores it back (`sb $v1,0x9($v0)`). It then
 * re-reads the cell (`lui $v1,0x1F80` / `lw $v1,0x44($v1)`) and tests the same
 * byte: while that byte is non-zero the handler steps the object's tint triplet
 * at 0x5D, 0x5E and 0x5F up by 7 each (`lbu`/`addiu 0x7`/`sb`), re-reading the
 * cell before the 0x5E and 0x5F channels so each of those two groups keeps its
 * own pointer load, and the 0x5F store sits in the `j` delay slot; when the byte
 * reads zero it instead stores 2 into the object's dispatch byte 0x01 (`sb` of
 * the constant the `beqz` delay slot loads), which hands the next frame to table
 * entry 2 (0x801F6FD0, invokeTwoHelpersScenarioScena1300_801F6FD0). Both arms
 * rejoin at the two overlay helper calls func_801F6FF8 and
 * setWorkByte29To5OnActiveWorkRecordsWithByte6SixScenarioScena1300_801F70DC, made
 * in that order with no arguments and with any result discarded, so the pair
 * runs on every call and their `jal` delay slots stay nops. The 0x18-byte frame
 * keeps only $ra at 0x10 across them, and the first pointer-cell load precedes
 * frame creation. No word in this payload encodes a `jal` to 0x801F6F24, so the
 * handler table is its only reference.
 *
 * Statement order, operand forms and the two separate pointer-cursor reloads
 * come from the exact mirror sibling
 * fadeWorkTintTripletBy6AndAdvanceWorkByte1To6WhenCountdownExpiresWorld02Area10814_801F3A6C
 * (0x801F3A6C, world02/area108/14), whose 0xAC-byte walk of the same record
 * differs only in the direction and size of its triplet step, the state byte it
 * stores on expiry and its two callees. This span's +7 step and its two `jal`
 * words appear in no other payload, so no byte-identical twin exists: the
 * discriminating immediates are its own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkCountdownAndRampTintTripletBy7OrAdvanceWorkByte2ThenInvokeTwoHelpersScenarioScena1300_801F6F24(void) {
  u32 slot_offset;
  u8 *countdown;
  u8 *work;

  slot_offset = 0x44u;
  countdown = PSX_REF(u8 *, SPAD_BASE + slot_offset);
  countdown[9]--;
  work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
  if (work[9] != 0) {
    work[0x5D] += 7;
    work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    work[0x5E] += 7;
    work = PSX_REF(u8 *, SPAD_BASE + slot_offset);
    work[0x5F] += 7;
  } else {
    work[1] = 2;
  }
  func_801F6FF8();
  setWorkByte29To5OnActiveWorkRecordsWithByte6SixScenarioScena1300_801F70DC();
}
