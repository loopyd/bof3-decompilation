#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. The cell is re-evaluated for
 * the second read group because the store through the loaded pointer may alias
 * it, so the compiler re-materializes `lui 0x1F80` / `lw 0x44` exactly where the
 * original does. Statement order, cell spelling and declaration style follow the
 * exact sibling incrementWorkBytes9AndAThenAdvanceWorkByte1At10BmagicMagic05503_801EEE3C
 * (0x801EEE3C), which reaches the same cell through SPAD_PTR_TABLE and keeps one
 * cell load for its final byte read-modify-write; this body carries only the
 * byte 0x09 increment before that group, and its own 0xC0 threshold. */

/* @source 0x801EEF0C
 * @behavior Overlay step handler for the scratchpad work object read through
 *           pointer cell 0x1F800044: increments the object unsigned byte 0x09 by
 *           one at byte width (lbu / addiu 1 / sb), then re-reads the pointer
 *           cell into a local pointer and, when the re-read unsigned byte 0x09
 *           equals 0xC0, increments work byte 0x01 through that same loaded
 *           pointer - the byte the in-payload dispatcher at 0x801EEC04 scales by
 *           four and uses to select its two-word handler run (0x801EEC5C,
 *           0x801EED40). The comparison is an equality on the re-read byte, not
 *           a bound on the value just stored, and it takes no sign extension or
 *           0xFF mask. Leaf with no frame; takes no arguments, returns nothing
 *           and reads or writes no other state. No jal in this payload and no
 *           in-image word names this span, so its caller is outside the archive.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte1AtC0BmagicMagic10703_801EEF0C(void)
{
  u8** slots;
  u8*  ptr;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x09] += 1;
  ptr = slots[0x11];
  if (ptr[0x09] == 0xC0) {
    ptr[1] += 1;
  }
}
