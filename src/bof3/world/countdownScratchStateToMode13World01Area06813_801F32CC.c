#include "bof3/bof3.h"

/* @source 0x801F32CC
 * @behavior Overlay step: reads the work pointer published at the scratchpad
 * pointer cell 0x1F800044, counts the record's byte at offset 0x09 down by one,
 * and, only when that byte was already zero, arms the record's state byte at
 * offset 0x01 with 13 - the index the local dispatchers
 * dispatchStateByte1Table3ff4World01Area06813 and
 * dispatchStateByte1Table4004World01Area06813 select with. The countdown store
 * happens on both paths, so it fills the spent-count branch's delay slot, and
 * the pointer cell is read a second time for the state store. Takes no
 * arguments, reads no other state, and returns nothing.
 * The original encodes the byte decrement as an unsigned-byte wrap add
 * (`addiu $v1, $a0, 0x00FF`), i.e. the countdown stores 0xFF when it was
 * already zero, hence the 0xFF term below. Shape evidence: the 56 bytes are the
 * same body as the exact world03/area134/13 family
 * countdownScratchStateToMode15World03Area13413_801F3254 (0x0F) and
 * countdownScratchStateToMode17World03Area13413_801F32E4 (0x11), which differ
 * only in that stored index; that family is defined by the shared
 * WORLD_DEFINE_COUNTDOWN_TO_MODE macro in include/world/countdown.h. This
 * target states the same shape inline because it owns no member of that shared
 * macro's source file.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownScratchStateToMode13World01Area06813_801F32CC(void) {
  u8** slots;
  u8* work;
  u8 value;

  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  value = work[0x09];
  work[0x09] = value + 0xFF;
  if (value == 0) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 13;
  }
}
