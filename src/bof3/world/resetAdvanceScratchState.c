#include "bof3/world/area01613_internal.h"

/* @source 0x801F3460
 * @behavior clears scratch-state offsets 0x09 and 0x0b, then increments 0x01.
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void resetAdvanceScratchState(void)
{
  u8** slots;
  u8*  ptr;

  slots = SPAD_PTR_TABLE(u8);

  slots[0x11][0x09] = 0;

  slots[0x11][0x0b] = 0;
  ptr = slots[0x11];
  ptr[1] += 1;
}
