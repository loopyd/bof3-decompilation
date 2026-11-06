#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. The cell is re-evaluated for
 * every store group because a store through the loaded pointer may alias it, so
 * the compiler re-materializes `lui 0x1F80` / `lw 0x44` exactly where the
 * original does. Statement order, cell spelling and declaration style follow the
 * exact sibling clearWorkBytes9ABAndAdvanceWorkByte1BmagicMagic10703_801EEE50
 * (0x801EEE50), which reaches the same cell through SPAD_PTR_TABLE and keeps one
 * cell load for its final byte read-modify-write. */

/* @source 0x801EEE3C
 * @behavior Step handler 1 of this overlay's three-word step run placed at
 *           0x801EEC04, whose words are 0x801EECCC, 0x801EEE3C and 0x801EEEA8.
 *           The per-frame dispatcher at 0x801EEC10 copies those words to its
 *           frame and calls the one selected by unsigned work byte 1, so this
 *           body has no jal caller. It increments the scratchpad work object's
 *           unsigned bytes 0x09 and 0x0A by one at byte width (lbu / addiu 1 /
 *           sb), re-reading the pointer cell 0x1F800044 before each store, then
 *           loads the cell once more and, when unsigned byte 0x09 equals 0x10,
 *           increments work byte 1 - the run index the dispatcher reads - so the
 *           step advances to handler 2 after 0x10 counts. Leaf with no frame; it
 *           takes no arguments, returns nothing and touches no other state. The
 *           comparison against the re-read byte takes no sign extension or
 *           0xFF mask, and the pointer cell load precedes nothing that could
 *           hide it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkBytes9AndAThenAdvanceWorkByte1At10BmagicMagic05503_801EEE3C(void)
{
  u8** slots;
  u8*  ptr;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x09] += 1;
  slots[0x11][0x0A] += 1;
  ptr = slots[0x11];
  if (ptr[0x09] == 0x10) {
    ptr[1] += 1;
  }
}
