#include "bof3/bof3.h"

/* @source 0x801F2C60
 * @behavior Clears the scratchpad work-object byte at offset 0x09 with a
 * zero byte store and then increments the work-object byte at offset 0x02 by
 * one. The pointer cell at 0x1F800044 is read once per access, so the original
 * re-materializes the cell address and reloads the work pointer for each of
 * the two accesses: the zero byte is stored through the first load and the
 * incremented byte through the second. The function takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9AndAdvanceWorkByte2World03Area14213_801F2C60(void) {
  u8 **slots;
  u8 *work;

  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][9] = 0;
  work = slots[0x11];
  work[2]++;
}
