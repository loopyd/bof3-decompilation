#include "bof3/bof3.h"

/* @source 0x801F2C6C
 * @behavior Overlay work-state handler: loads the scratchpad work object
 *           published at pointer-slot 0x1F800044, arms its countdown halfword at
 *           offset 0x2C with 0x20, clears its origin words at offsets 0x64, 0x68
 *           and 0x6C, then advances its dispatch byte at offset 0x01 by one; it
 *           takes no arguments and returns nothing. Field evidence comes from
 *           this payload's own neighbour at 0x801F2C9C, which decrements that
 *           +0x2C halfword each frame, adds 4 to the words at +0x64 and +0x68,
 *           re-arms +0x2C with 0x10 and advances byte 0x01 once the countdown
 *           expires; the already-exact scenario lift func_801F85A8 reads the
 *           object's +0x64/+0x68/+0x6C as one origin VECTOR. The address is
 *           entry 0 of the per-frame dispatch table at 0x801F5734 that this
 *           payload's entry point 0x801F2C04 indexes with work byte 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkCountdownClearOriginIncrementWorkByte1World02Area08213_801F2C6C(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u16*)(work + 0x2C) = 0x20;
  *(u32*)(work + 0x64) = 0;
  *(u32*)(work + 0x68) = 0;
  *(u32*)(work + 0x6C) = 0;
  work[1]++;
}
