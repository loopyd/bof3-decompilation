#include "bof3/bof3.h"
#include "world/countdown.h"

/* @source 0x801F3254
 * @behavior Overlay state handler: reads the work pointer published at the
 * scratchpad cell 0x1F800044, counts the record's byte at offset 0x09 down by
 * one, and, when that byte was already zero, arms the record's state byte at
 * offset 0x01 with 0x0F. The countdown store happens on both paths, so it fills
 * the spent-count branch's delay slot, and the pointer cell is read a second
 * time for the state store. Takes no arguments, reads no other state, and
 * returns nothing.
 * The original encodes the byte decrement as an unsigned-byte wrap add
 * (`addiu $v1, $a0, 0x00FF`), i.e. the countdown stores 0xFF when it was
 * already zero, hence the 0xFF term below.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_COUNTDOWN_TO_MODE(countdownScratchStateToMode15World03Area13413_801F3254, 0x0F)

/* @source 0x801F32E4
 * @behavior Overlay state handler: reads the work pointer published at the
 * scratchpad cell 0x1F800044, counts the record's byte at offset 0x09 down by
 * one, and, when that byte was already zero, arms the record's state byte at
 * offset 0x01 with 0x11. The countdown store happens on both paths, so it fills
 * the spent-count branch's delay slot, and the pointer cell is read a second
 * time for the state store. Takes no arguments, reads no other state, and
 * returns nothing.
 * The original encodes the byte decrement as an unsigned-byte wrap add
 * (`addiu $v1, $a0, 0x00FF`), i.e. the countdown stores 0xFF when it was
 * already zero, hence the 0xFF term below.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_COUNTDOWN_TO_MODE(countdownScratchStateToMode17World03Area13413_801F32E4, 0x11)

/* @source 0x801F3B7C
 * @behavior Overlay state handler: reads the work pointer published at the
 * scratchpad cell 0x1F800044, counts the record's byte at offset 0x09 down by
 * one, and, when that byte was already zero, arms the record's state byte at
 * offset 0x01 with 0x0F. The countdown store happens on both paths, so it fills
 * the spent-count branch's delay slot, and the pointer cell is read a second
 * time for the state store. Takes no arguments, reads no other state, and
 * returns nothing.
 * The original encodes the byte decrement as an unsigned-byte wrap add
 * (`addiu $v1, $a0, 0x00FF`), i.e. the countdown stores 0xFF when it was
 * already zero, hence the 0xFF term below.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_COUNTDOWN_TO_MODE(countdownScratchStateToMode15World03Area13413_801F3B7C, 0x0F)

/* @source 0x801F3C0C
 * @behavior Overlay state handler: reads the work pointer published at the
 * scratchpad cell 0x1F800044, counts the record's byte at offset 0x09 down by
 * one, and, when that byte was already zero, arms the record's state byte at
 * offset 0x01 with 0x11. The countdown store happens on both paths, so it fills
 * the spent-count branch's delay slot, and the pointer cell is read a second
 * time for the state store. Takes no arguments, reads no other state, and
 * returns nothing.
 * The original encodes the byte decrement as an unsigned-byte wrap add
 * (`addiu $v1, $a0, 0x00FF`), i.e. the countdown stores 0xFF when it was
 * already zero, hence the 0xFF term below.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_COUNTDOWN_TO_MODE(countdownScratchStateToMode17World03Area13413_801F3C0C, 0x11)
