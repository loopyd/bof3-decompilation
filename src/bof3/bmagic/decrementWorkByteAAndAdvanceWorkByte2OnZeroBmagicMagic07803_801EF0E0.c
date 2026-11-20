#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the exact sibling clearWorkByteAAndIncrementWorkByte2WhenGlobalWorkByteBIs2
 * BmagicMagic07803_801EF228 uses. The original re-materializes the cell address
 * before each of the two accesses on the decrement path (lui $v1,0x1F80 / lw
 * $v1,0x44($v1)), so both statements stay on the named cell instead of sharing
 * one loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0E0
 * @behavior Entry 3 of this overlay's nine-word work-byte-1 handler pointer run
 *           based at 0x801F0878, whose dispatcher at 0x801EEEE4 indexes that run
 *           on the work-object byte at offset 0x01. Decrements the unsigned
 *           byte 0xA of the scratchpad work object reached through the pointer
 *           cell at 0x1F800044 as a byte-width read-modify-write (lbu / addiu
 *           -1 / sb) and tests the stored value against zero (andi 0xFF / bnez,
 *           empty delay slot). Only when that byte reaches zero it
 *           re-materializes the cell with its own lui/lw pair and increments the
 *           object's byte 2 at byte width (lbu / addiu 1 / sb), which moves the
 *           dispatcher to its next step. The second read of the cell is
 *           required: the store through the loaded work pointer may alias the
 *           cell. Takes no arguments, returns nothing, keeps no frame and reads
 *           or writes no other state. Bytes 0x801EF0E0..0x801EF12B were
 *           byte-identical to the family sibling
 *           decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic06203_801EFC00
 *           when this body was written (a payload cmp over out/binaries found
 *           the same 76-byte span in magic067, magic082, magic111, magic131,
 *           magic137 and magic222 as well), and this body follows that
 *           sibling's spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic07803_801EF0E0(void) {
  if (--g_battle_work[0xA] == 0) {
    g_battle_work[2]++;
  }
}
