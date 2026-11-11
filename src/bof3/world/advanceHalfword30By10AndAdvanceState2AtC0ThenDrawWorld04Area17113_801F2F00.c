#include "bof3/bof3.h"

void func_801F3070(s32 arg0);

/* @source 0x801F2F00
 * @behavior Per-frame byte-2 handler of the overlay work record published at
 *           the scratchpad pointer cell 0x1F800044, reached through the
 *           handler run based at 0x801F3F5C: it is word 2 of that run, whose
 *           five words are 0x801F2CA0, 0x801F2E1C, this address, 0x801F2F5C
 *           and 0x801F3000, and the overlay dispatcher at 0x801F2C5C calls the
 *           word the record state byte at 0x02 selects, so no jal names it.
 *           Adds 0x10 to the record's unsigned halfword at 0x30, stores the
 *           truncated halfword, and compares the value that store wrote, read
 *           back as a signed halfword: once it reaches 0xC0 the record state
 *           byte at 0x02 advances by one, so the next dispatch takes entry 3.
 *           It then calls the payload draw routine at 0x801F3070 with argument
 *           0, that routine's base-only stage, which re-reads the same record,
 *           projects its route halfwords into the scratchpad pair and builds
 *           its sprite primitive; a non-zero argument there also runs its extra
 *           stage. The cell is loaded once and that one base serves both the
 *           halfword update and the byte store. Takes no arguments and returns
 *           nothing; its 0x18-byte frame only holds $ra at 0x10 across the
 *           call, whose delay slot zeroes $a0, and the slots after the halfword
 *           load and after the branch stay nops.
 *
 *           Shape evidence: this 0x5C span was compared word for word with the
 *           exact body of
 *           advanceHalfword30By10AndAdvanceState2AtC0ThenDrawWorld04Area17013_801F3F28.
 *           Of its 23 words only two differ: the jal at +0x44, and the delay
 *           slot at +0x48 that passes 0 here and stays empty there. Each jal
 *           names its own payload's draw routine, so the spans are
 *           relocation-different and this one is not a clone of a lifted
 *           sibling. The copies of this body in emi/world04/area169/13 at
 *           0x801F2F0C and in emi/world03/area117/13 at 0x801F2F00 likewise
 *           differ in their own jal words (and area117's in its step and limit
 *           constants), so they supply shape only. This one copies that
 *           sibling's statement order, offset-local spelling and halfword
 *           read-modify-write form, and writes this span's own literals, call
 *           target and call argument from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By10AndAdvanceState2AtC0ThenDrawWorld04Area17113_801F2F00(
    void) {
  u8* record;
  s16* half;

  record = SPAD_PTR_SLOT(u8, 0x44u);
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x10);
  if (*half >= 0xC0) {
    record[2]++;
  }
  func_801F3070(0);
}
