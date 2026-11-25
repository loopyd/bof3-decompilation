#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is a named cell
 * symbol, the form the exact siblings use: the symbolic address lets the
 * assembler fold it into each load (lui $vN,0x1F80 / lw $vN,68($vN)). This body
 * stores through the first loaded pointer before it reads the byte back, so the
 * cell is materialised a second time for the test and its reloaded pointer
 * carries the byte-2 increment. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F00A0
 * @behavior Overlay step handler, entry 13 of the fifteen in-image words of the
 *           handler-pointer run based at 0x801F04D0: the word at 0x801F0500 is
 *           the only word in this payload that holds this address, so no jal
 *           names the body and the table row proves the entry boundary and the
 *           no-argument ABI. It first reads the scratchpad work-object pointer
 *           cell at 0x1F800044 (lui / lw) and adds two to work-object byte 9
 *           modulo 256 (lbu / addiu 2 / sb). It then materialises the same cell
 *           again with its own lui / lw pair, re-reads the byte it just stored
 *           through that reload, and only when that byte equals 0x10 increments
 *           work-object byte 2 modulo 256 (lbu / addiu 1 / sb), advancing to the
 *           next byte-2 handler step. Leaf with no frame and no saved registers:
 *           it takes no arguments, returns nothing through the jr $ra at
 *           0x801F00EC, and reads or writes no other state. The roles of
 *           work-object bytes 9 and 2 are not established and stay
 *           offset-anchored.
 *
 *           A search of every payload under out/binaries finds one second copy
 *           of this 0x50-byte span, at 0x801F038C of emi/bmagic/magic092/03,
 *           and a payload cmp of the two spans reports zero differing bytes, so
 *           the whole body is common to both overlays. That copy still sits on
 *           its asm row, so it is span proof and not a clone source, and the
 *           clone lever is declined for want of a lifted twin. The statement
 *           order, the second cell materialisation and the reload discipline
 *           come instead from the reviewed exact lift add4ToWorkByte9AndAdvance
 *           WorkByte2At11BmagicMagic09203_801EF940, whose 0x50-byte body differs
 *           in two words alone: its add-immediate is four here two, and its
 *           equality test compares 0x11 here 0x10. The near neighbours
 *           incrementWorkByte2WhenGlobalWorkByteBFlag80SetBmagicMagic14403
 *           _801F00F0 and runReadinessHelperThenAdd4ToWorkByte9AndAdvanceWork
 *           Byte2At10BmagicMagic17303_801F0648 guard on other state or keep the
 *           byte in a register, so neither supplies this body. The literals,
 *           the reload point and the missing third cell load are this span's
 *           own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workByte9AdvanceAt10(void) {
  g_battle_work[9] += 2;
  if (g_battle_work[9] == 0x10) {
    g_battle_work[2]++;
  }
}
