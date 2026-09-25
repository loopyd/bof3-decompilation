#include "bof3/bof3.h"

/* @source 0x801F3494
 * @behavior clears scratch-state offsets 0x09 and 0x0b, then increments 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetAdvanceScratchStateCloneWorld02Area08713_801F3494(void)
{
  u8** slots;
  u8*  ptr;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x09] = 0;

  slots[0x11][0x0b] = 0;
  ptr = slots[0x11];
  ptr[1] += 1;
}
