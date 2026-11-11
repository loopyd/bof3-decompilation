#include "bof3/bof3.h"

void func_801F3F84(void);

/* @source 0x801F3DA8
 * @behavior Byte-2 state handler entry 2 of the five-entry handler table based
 *           at 0x801F4448, the table the overlay dispatcher at 0x801F3A8C
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044: subtracts 0x20
 *           from that record unsigned halfword at offset 0x30, stores the
 *           truncated halfword, and, once the stored value read as a signed
 *           halfword falls below -0xFF, increments the record state byte at
 *           offset 0x02 so the next dispatch takes entry 3 at 0x801F3E04. It
 *           then calls the payload record draw routine at 0x801F3F84, which
 *           imports SetDrawMode, SetPolyFT4, SetShadeTex and RotTransPers4.
 *           Takes no arguments and returns nothing; the record pointer is loaded
 *           before the frame is created, the 0x18-byte frame only keeps $ra at
 *           0x10 across the call, and the slots after the halfword load and
 *           after the branch stay nops.
 *
 *           A little-endian word compare of this 0x5C span against the payloads
 *           found the same body in seven other overlays and measured each
 *           difference as exactly one word, the jal at +0x44: World03/Area11713
 *           0x801F2F00 calls 0x801F314C, World03/Area11913 0x801F3114 calls
 *           0x801F3324, World03/Area14013 0x801F2E64 calls 0x801F314C and
 *           World04/Area15813, Area16113, Area16213 and Area16613 at 0x801F2EF8
 *           call 0x801F30D4, each target's own draw routine. The spans are
 *           relocation-different rather than equal, so none of them was a clone
 *           source; the statement order, operand forms and offset spelling came
 *           from the World03/Area11713 body, whose 0x5C span measured exact.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By20AndAdvanceWorkByte2BelowMinusFfThenDrawWorld03Area13913_801F3DA8(
    void) {
  u8* work;
  u16 half;

  work = SPAD_PTR_SLOT(u8, 0x44);
  half = *(u16*)(work + 0x30) - 0x20;
  *(u16*)(work + 0x30) = half;
  if ((s16)half < -0xFF) {
    work[2]++;
  }
  func_801F3F84();
}
