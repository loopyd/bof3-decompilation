#include "bof3/bof3.h"

/* @source 0x801E3BAC
 * @behavior clears byte four then sets byte two in the scratchpad work target.
 */
void battle03_reset_scratch_byte4_set2(void) {
  u8** scratch_slots;
  u8            value;

  scratch_slots = SPAD_PTR_TABLE(u8);
  scratch_slots[0x11][4] = 0;
  scratch_slots = SPAD_PTR_TABLE(u8);
  value = 1;
  scratch_slots[0x11][2] = value;
}
