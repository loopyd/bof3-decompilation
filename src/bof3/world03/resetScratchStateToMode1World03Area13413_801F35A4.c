#include "bof3/bof3.h"

/* @source 0x801F35A4
 * @behavior Resets the scratchpad work object published at pointer cell
 * 0x1F800044 for a fresh mode-1 pass: clears the object's state byte at offset
 * 0x09, arms the mode/state byte at offset 0x01 with 1 and clears the object's
 * byte at offset 0x02. The pointer cell is re-read before every store, so each
 * of the three stores uses its own load of 0x1F800044 and the last store fills
 * the $ra delay slot. It takes no arguments, returns nothing and reads no
 * other state. The name mirrors the exact sibling resetScratchSelectMode1 at
 * 0x801F3288, which performs the same offset 0x09 clear plus offset 0x01 arm
 * for its own overlay work object.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetScratchStateToMode1World03Area13413_801F35A4(void) {
  u8** slots;
  u8 value;

  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][0x09] = 0;
  slots = SPAD_PTR_TABLE(u8);
  value = 1;
  slots[0x11][0x01] = value;
  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][0x02] = 0;
}
