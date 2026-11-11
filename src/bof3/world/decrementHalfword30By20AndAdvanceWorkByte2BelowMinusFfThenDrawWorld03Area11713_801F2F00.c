#include "bof3/bof3.h"

void func_801F314C(void);

/* @source 0x801F2F00
 * @behavior Byte-2 state handler entry 2 of the six-entry handler table based
 *           at 0x801F4624, the table the overlay dispatcher at 0x801F2C1C
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044: subtracts 0x20
 *           from that record's unsigned halfword at offset 0x30, stores the
 *           truncated halfword, and, once the stored value read as a signed
 *           halfword falls below -0xFF, increments the record state byte at
 *           offset 0x02 so the next dispatch takes entry 3 at 0x801F2F5C. It
 *           then calls the payload record draw routine at 0x801F314C, which
 *           builds the record quad (SetDrawMode, SetPolyFT4, SetShadeTex,
 *           RotTransPers4) and reads the same halfword at offset 0x30 as a
 *           sprite screen offset. Takes no arguments and returns nothing; the
 *           record pointer is loaded before the frame is created, the 0x18-byte
 *           frame only keeps $ra at 0x10 across the call, and the slots after
 *           the halfword load and after the branch stay nops.
 *
 *           This is the World03/Area11913 twin 0x801F3114 of the same overlay
 *           family, proven one body by a payload compare of the two 0x5C spans:
 *           they differ in exactly one word, the `jal` at +0x44, whose targets
 *           0x801F314C here and 0x801F3324 there are each their own payload's
 *           draw routine, so the spans are relocation-different and this one is
 *           not a clone of a lifted sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword30By20AndAdvanceWorkByte2BelowMinusFfThenDrawWorld03Area11713_801F2F00(
    void) {
  u8* work;
  u16 half;

  work = SPAD_PTR_SLOT(u8, 0x44);
  half = *(u16*)(work + 0x30) - 0x20;
  *(u16*)(work + 0x30) = half;
  if ((s16)half < -0xFF) {
    work[2]++;
  }
  func_801F314C();
}
