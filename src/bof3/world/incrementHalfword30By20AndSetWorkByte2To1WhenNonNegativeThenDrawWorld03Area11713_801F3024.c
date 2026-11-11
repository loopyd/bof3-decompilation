#include "bof3/bof3.h"

void func_801F2E08(void);
void func_801F314C(void);
extern u8* D_1F800044;
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */
extern void func_8015DF18(u32 cue_id); /* @source 0x8015DF18 @kind function */

/* @source 0x801F3024
 * @behavior Byte-2 state handler entry 4 of the six-entry handler table based
 *           at 0x801F4624, the table the overlay dispatcher at 0x801F2C1C
 *           indexes by the state byte at offset 0x02 of the work record
 *           published at the scratchpad pointer cell 0x1F800044: adds 0x20 to
 *           that record's unsigned halfword at offset 0x30 and stores the
 *           truncated halfword. When the stored value read as a signed
 *           halfword is nonnegative it calls the overlay camera-window handler
 *           at 0x801F2E08, reloads the record pointer, and returns without
 *           drawing when the state byte of the reloaded record reads 2, which
 *           is the state that handler may have just stored. Otherwise it
 *           stores 1 into the state byte so the next dispatch takes entry 1 at
 *           0x801F2E08, and queues cue 0x201 through the resident routine
 *           0x8015DF18 when the shared phase byte at 0x80143BB0 reads zero.
 *           Every path except that early return finishes by calling the
 *           payload record draw routine at 0x801F314C, which reads the same
 *           halfword at offset 0x30 as a sprite screen offset. Takes no
 *           arguments and returns nothing; the record pointer is loaded before
 *           the frame is created, the 0x18-byte frame only keeps $ra at 0x10
 *           across the calls, and the slots after the halfword load, after the
 *           signed test and after the phase-byte test stay nops.
 *
 *           The record pointer cell is named in its D_<ADDRESS> form because
 *           this target's map binds no name for it: the original materialises
 *           the cell address separately at each of its two reads, and the
 *           constant-address dereference spelling of the same cell made the
 *           compiler keep one base in a callee-saved register instead.
 *
 *           This is the World03/Area11913 twin 0x801F3210 of the same overlay
 *           family, lifted exact there. A payload compare of the two 0x88 spans
 *           at their own offsets (0x424 here, 0x610 there) differs in exactly
 *           two words: the jal at +0x2C, whose targets 0x801F2E08 here and
 *           0x801F301C there are each their own payload's camera-window
 *           handler, and the jal at +0x70, whose targets 0x801F314C here and
 *           0x801F3324 there are each their own payload's draw routine. The two
 *           deltas differ (+0x214 and +0x1D8), so the spans are relocation
 *           different and this one is not a clone of the lifted sibling; the
 *           sibling supplied the statement order, operand forms and name while
 *           every callee binding came from this payload's own words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementHalfword30By20AndSetWorkByte2To1WhenNonNegativeThenDrawWorld03Area11713_801F3024(
    void) {
  u8* work;
  u8* resumedWork;
  u16 half;

  work = D_1F800044;
  half = *(u16*)(work + 0x30) + 0x20;
  *(u16*)(work + 0x30) = half;
  if ((s16)half >= 0) {
    func_801F2E08();
    resumedWork = D_1F800044;
    if (resumedWork[2] == 2) {
      return;
    }
    resumedWork[2] = 1;
    if (D_80143BB0 == 0) {
      func_8015DF18(0x201);
    }
  }
  func_801F314C();
}
