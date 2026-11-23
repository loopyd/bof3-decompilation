#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044 as g_battle_work, the name the exact
 * lifts of this same target use (copyWorkWordTripletClearByte9ThenAdvanceWork
 * Byte2BmagicMagic00603_801F05A4 and decrementWorkByte9ThenCopyRecordTriplet
 * Add800000ToWord3CSetDirectionAndAngleToward8014638CRearmByte9AndAdvanceWork
 * Byte2BmagicMagic00603_801EEF28). The original re-materialises the cell with
 * its own lui 0x1F80 plus lw 44($vN) pair at every read that follows a store
 * through the loaded pointer, which may alias the cell, so the entry read is
 * kept once for the two accesses that share it and the two later reads inside
 * the tail are written as fresh cell reads. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF04C
 * @behavior Entry index 1 of the thirteen-word handler run
 *           0x801EEC04..0x801EEC34 that this overlay indexes with work byte 2;
 *           no jal in the payload targets this body and no other word of the
 *           image holds it, so that table is its only route and the parameter
 *           roles stay structural. It reads the scratchpad work-object pointer
 *           cell at 0x1F800044 once and, through that one pointer, advances the
 *           object's word at offset 0x14 by 0x80 (lw $v0,0x14($a0) / addiu
 *           $v0,$v0,0x80 / sw, the store the original writes in the
 *           taken/not-taken branch delay slot, so it runs on both paths) and
 *           decrements the object's unsigned byte at offset 9 (lbu / addiu
 *           $v1,$v1,-1 / sb). It returns without more work while that new
 *           count is non-zero, the guard masking the kept register with andi
 *           0xFF before bnez rather than reloading the byte. On zero it
 *           re-reads the cell, sets that same byte 9 to the object's unsigned
 *           byte at offset 0xB plus 0xC (lbu $v0,0xB / addiu $v0,$v0,0xC /
 *           sb) and, through a third cell read, increments the object's byte
 *           at offset 0x2 (lbu / addiu $v0,$v0,1 / sb), advancing the
 *           step selector so the next frame runs entry 2. Takes no arguments,
 *           returns nothing through the jr $ra at 0x801EF0B0, keeps no frame
 *           and writes no other state. The roles of work bytes 2, 9 and 0xB
 *           and of work word 0x14 are not established beyond what this body
 *           does to them, so they stay offset-anchored, and 0x80 and 0xC are
 *           written as the literal constants they are.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCountDownPlus(
    void) {
  u8 *work;
  s32 value;

  work = g_battle_work;
  value = *(s32 *)(work + 0x14) + 0x80;
  *(s32 *)(work + 0x14) = value;
  if (--work[9] == 0) {
    u8 *rearm;

    rearm = g_battle_work;
    rearm[9] = rearm[0xB] + 0xC;
    g_battle_work[2]++;
  }
}
