#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This target's map binds it at 0x1F800044 as g_battle_work, the same binding the
 * sibling sources decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic07803_
 * 801EF0E0 and incrementWorkByte9Add2ToWorkByteAThenAdvanceWorkByte2At20Bmagic_
 * Magic05903_801EF504 carried when this body was written, both measured exact
 * then. The original re-materializes the cell address with its own
 * lui $vN,0x1F80 / lw $vN,0x44($vN) pair before each of its two reads, the
 * second following the byte store through the first loaded pointer. The named
 * cell supplies the first pair by itself for the byte read-modify-write, and the
 * one local pointer below carries the second pair and the guarded advance, which
 * is why the two reads land in different registers. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF278
 * @behavior Entry 7 of this overlay's nine-word work-byte-1 handler pointer run
 *           based at 0x801F0878, whose dispatcher at 0x801EEEE4 indexes that run
 *           on the work-object byte at offset 0x01. It adds eight to the
 *           scratchpad work object's unsigned byte 0xA reached through the
 *           pointer cell at 0x1F800044 as a byte-width read-modify-write (lbu /
 *           addiu 8 / sb), re-materializes the cell with its own lui/lw pair and
 *           tests the re-read byte against an unsigned bound of 0x81 (lbu /
 *           sltiu 0x81 / bnez to the return); only when that byte is at least
 *           0x81 it increments the object's byte 2 at byte width (lbu / addiu 1 /
 *           sb) through the pointer it already holds, which moves the dispatcher
 *           to its next step. The re-read before the test is required: the store
 *           through the loaded work pointer may alias the cell. Takes no
 *           arguments, returns nothing, keeps no frame and reads or writes no
 *           other state. No byte-identical sibling exists: a payload cmp over
 *           every file under out/binaries found this 84-byte span once, in this
 *           target's own payload. The family sibling incrementWorkByte9Add2To
 *           WorkByteAThenAdvanceWorkByte2At20BmagicMagic05903_801EF504 (exact
 *           when this body was written) supplied the statement order, operand
 *           forms and reload points; this span's own bytes supply its single
 *           byte-0xA step, its 0x81 bound and the advance through the pointer the
 *           test already loaded.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workByteAdvanceLeast81(void) {
  u8 *work;

  g_battle_work[0xA] = g_battle_work[0xA] + 8;
  work = g_battle_work;
  if (work[0xA] >= 0x81) {
    work[2] = work[2] + 1;
  }
}
