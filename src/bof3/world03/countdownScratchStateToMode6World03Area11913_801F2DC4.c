#include "bof3/bof3.h"

/* @source 0x801F2DC4
 * @behavior Overlay state handler: reads the work pointer published at the
 * scratchpad cell 0x1F800044, counts the record's byte at offset 0x09 down by
 * one, and, when that byte was already zero, arms the record's state byte at
 * offset 0x01 with 0x06. The countdown store happens on both paths, so it fills
 * the spent-count branch's delay slot, and the pointer cell is read a second
 * time for the state store. The shape is the exact sibling of
 * countdownScratchStateToMode15World03Area13413_801F3254 in this module, and it
 * sits 0x34 bytes after the same-shape mode-0x03 handler at 0x801F2CE0, which
 * differs only in the state byte value it arms. Takes no arguments, reads no
 * other state, and returns nothing.
 * The original encodes the byte decrement as an unsigned-byte wrap add
 * (`addiu $v1, $a0, 0x00FF`), i.e. the countdown stores 0xFF when it was
 * already zero, hence the 0xFF term below.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownScratchStateToMode6World03Area11913_801F2DC4(void) {
  u8** slots;
  u8* work;
  u8 value;

  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  value = work[0x09];
  work[0x09] = value + 0xFF;
  if (value == 0) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x06;
  }
}
