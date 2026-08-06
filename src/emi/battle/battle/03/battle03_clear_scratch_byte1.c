#include "bof3/bof3.h"

/* @source 0x801E6FA0
 * @behavior clears byte one in the current scratchpad work record.
 */
void battle03_clear_scratch_byte1(void) {
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 0;
}
