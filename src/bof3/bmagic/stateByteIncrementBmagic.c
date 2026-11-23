#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This target's map binds it at 0x1F800044 and the siblings
 * setWorkByteATo15AndAdvanceWorkByte2WhenHeadingBucketMatchesRecordToward8014638C
 * BmagicMagic07803_801EF12C and add8ToWorkByteAThenAdvanceWorkByte2WhenAtLeast81
 * BmagicMagic07803_801EF278 carried the same binding when their bodies were
 * written, both measured exact then. The original materializes the cell with a
 * lui $a0,0x1F80 / lw $a0,0x44($a0) pair for its first read and a second
 * lui $v1 / lw pair after the byte-0xA store, so the two reads land in different
 * registers; the named cell supplies both pairs. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF08C
 * @behavior Entry 3 of this overlay's nine-word work-byte-1 handler pointer run
 *           based at 0x801F0878, whose dispatcher at 0x801EEEE4 indexes that run
 *           on the work-object byte at offset 0x01. It loads the scratchpad work
 *           object once through the pointer cell at 0x1F800044 and reads that
 *           object's unsigned byte 9 (lbu / andi 0xFF / addiu 0xC / bne). When
 *           the byte equals 12 it writes 15 to the object's byte 0xA, then
 *           re-materializes the cell with its own lui/lw pair and increments the
 *           object's byte 2 at byte width (lbu / addiu 1 / sb), which moves the
 *           dispatcher to its next step; on any other value it stores byte 9
 *           plus one back to byte 9 and returns. The reload is required: the
 *           store through the loaded work pointer may alias the cell. Takes no
 *           arguments, returns nothing, keeps no frame and reads or writes no
 *           other state. No byte-identical sibling exists: a payload cmp over
 *           every file under out/binaries found this 84-byte span once, in this
 *           target's own payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateByteIncrementBmagic(void) {
  u8 *work;
  u8 value;

  work = g_battle_work;
  value = work[9];
  if ((value & 0xFF) == 0xC) {
    work[0xA] = 15;
    g_battle_work[2]++;
  } else {
    work[9] = value + 1;
  }
}
