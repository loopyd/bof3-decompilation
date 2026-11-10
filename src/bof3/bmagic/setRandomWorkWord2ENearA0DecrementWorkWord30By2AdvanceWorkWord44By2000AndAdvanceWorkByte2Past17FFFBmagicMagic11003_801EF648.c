#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-object pointer cell, bound by this target's own map row
 * g_battle_work = 0x1F800044 and spelled the same way by this target's
 * existing exact lifts. The body reads the cell once, after the rand() call,
 * and addresses every work record field through that one loaded base
 * (lui $a1,0x1F80 / lw $a1,0x44($a1) at 0x801EF664), so no access
 * re-materialises it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF648
 * @behavior Entry 5 (index 5 of fourteen words) of the byte-1 handler table
 *           based at 0x801F046C, which
 *           dispatchStateByte1Table046cBmagicMagic11003_801EF3F8 indexes with
 *           work record byte 1 and calls indirectly; the word at 0x801F0480
 *           holds this body's address, and entry 4 at 0x801EF528 is the
 *           seeder that writes the two halfwords this body steps. It calls
 *           rand(), stores the low two bits of the answer plus 0x9F into the
 *           unsigned halfword at 0x2E (andi 3 / addiu 0x9F / sh), subtracts
 *           two from the unsigned halfword at 0x30 with a halfword-width
 *           read-modify-write (lhu / addiu -2 / sh), then advances the signed
 *           word at 0x44 by 0x2000 and stores the advanced word back
 *           unconditionally, that store filling the taken/not-taken branch
 *           delay slot. Only when the new word is greater than 0x17FFF under a
 *           signed compare (slt against a register holding 0x17FFF) does it
 *           increment work record byte 2 with a byte-width add (lbu / addiu 1 /
 *           sb), advancing the work-byte-2 dispatcher to its next handler step.
 *           It reads the scratchpad work pointer cell at 0x1F800044 once and
 *           after the call, so the base stays in $a1 and needs no callee-saved
 *           register: the frame is $ra-only at 0x18 and the prologue is just
 *           addiu $sp,-0x18 / sw $ra,0x10($sp). Takes no arguments, returns
 *           nothing and reads or writes no other state. Bytes
 *           0x801EF648..0x801EF6B3 carry no byte-identical twin: the
 *           distinguishing immediates 0x17FFF, 0x9F and 0x2000 appear in no
 *           other payload or lifted source, so the body is seeded from the
 *           nearest exact shape sibling,
 *           incrementWorkWordCAndAdvanceWorkByte2At2EBmagicMagic12103_801EF324,
 *           whose statement order (advance a work word into a local, store it
 *           unconditionally, then test the stored value and increment work byte
 *           2) and declaration style are kept while every literal, offset and
 *           width here comes from this span's own bytes; the scratchpad cell
 *           keeps this target's own mapped spelling g_battle_work.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRandomWorkWord2ENearA0DecrementWorkWord30By2AdvanceWorkWord44By2000AndAdvanceWorkByte2Past17FFFBmagicMagic11003_801EF648(
    void) {
  s32 roll;
  u8 *work;
  s32 value;

  roll = rand();
  work = g_battle_work;
  *(u16 *)(work + 0x2E) = (roll & 3) + 0x9F;
  *(u16 *)(work + 0x30) -= 2;
  value = *(s32 *)(work + 0x44) + 0x2000;
  *(s32 *)(work + 0x44) = value;
  if (value > 0x17FFF) {
    work[2]++;
  }
}
