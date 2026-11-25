#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes the cell address for every
 * access (lui $v1,0x1F80 / lw $v1,0x44($v1)), so the cell is reached through
 * this target-map symbol rather than through a constant address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFD50
 * @behavior Per-frame step of the work-object handler this overlay dispatches
 * on work-object byte 2: decrements the unsigned byte 0xA of the scratchpad
 * work object reached through the pointer cell at 0x1F800044 with a byte-width
 * read-modify-write (lbu / addiu -1 / sb) and compares the stored byte
 * truncated to eight bits with zero (andi 0xFF / bnez). Only when that byte
 * reaches zero does it re-read the cell and increment the object's byte 2 with
 * a byte-width add, advancing the dispatcher at 0x801EFC58 to the next byte-2
 * handler step. The second read of the cell is in the original: the store
 * through the loaded work pointer may alias the cell. It is entry index 2 of
 * the four-word work-byte-2 step table at 0x801F0114, takes no arguments,
 * returns nothing, keeps no frame and reads or writes no other state. Bytes
 * 0x801EFD50..0x801EFD9B are byte-identical to
 * decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic08203_801EFD74, whose
 * body spelling this follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCountdownZeroStep(void) {
  if (--g_battle_work[0xA] == 0) {
    g_battle_work[2]++;
  }
}
