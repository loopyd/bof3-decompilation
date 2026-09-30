#include "bof3/bof3.h"

/* @source 0x801F2CF0
 * @behavior Overlay per-frame step handler: loads the scratch work record
 *           cursor from scratchpad pointer-slot 0x1F800044 and decrements the
 *           unsigned 16-bit countdown held at the record's +0x2C field; while
 *           the decremented value is non-zero it is stored back and the handler
 *           returns. Only when the countdown reaches zero does it write the
 *           halfword 0xC into +0x2C (re-arming the countdown) and advance the
 *           record's +0x01 dispatch byte by one; it takes no arguments and
 *           returns nothing. The +0x2C field is the same countdown the
 *           already-exact sibling setWorkCountdownClearOriginIncrementWorkByte1
 *           World02Area08213_801F2C6C arms with 0x20 in this payload, and this
 *           address is entry 1 of the per-frame dispatch table at 0x801F5734
 *           that the payload entry point 0x801F2C04 indexes with work byte
 *           0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementCountdownRearmTo12IncrementWorkByte1World02Area08213_801F2CF0(
    void) {
  u8* work;
  u16 countdown;

  work = SPAD_PTR_SLOT(u8, 0x44);
  countdown = *(u16*)(work + 0x2C);
  countdown = (u16)(countdown - 1);
  *(u16*)(work + 0x2C) = countdown;
  if (countdown == 0) {
    work[1]++;
    *(u16*)(work + 0x2C) = 0xC;
  }
}
