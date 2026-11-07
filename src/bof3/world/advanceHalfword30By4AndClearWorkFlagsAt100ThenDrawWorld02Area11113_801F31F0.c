#include "bof3/bof3.h"

void func_80196070(void);
void func_801F3244(void);

/* @source 0x801F31F0
 * @behavior Last of the three byte-2 state handlers dispatched through the
 *           table based at 0x801F489C for the work record published at the
 *           scratchpad pointer cell 0x1F800044. Adds 4 to the record's
 *           unsigned halfword at offset 0x30 and stores the truncated
 *           halfword; once that stored value reads as a signed halfword at
 *           least 0x100, the step clears the shared work flags through the
 *           exe-resident helper at 0x80196070 (the handler index at offset
 *           0x04 goes back to zero). Either way it then calls the overlay
 *           draw routine at 0x801F3244, which every handler of this table
 *           ends with. Takes no arguments and returns nothing; the 0x18-byte
 *           frame only keeps $ra at 0x10 across the calls.
 *
 * Name evidence: the offset, step and limit come from the original
 * `lhu/addiu 4/sh` plus the `sll 16`/`sra 16`/`slti 0x100` sequence; the
 * helper at 0x80196070 is the already-lifted clearWorkFlags body, which
 * writes only the record's bytes 0x00-0x04; and the handler table entry is
 * proven by the three in-payload pointers at 0x801F489C/0x801F48A0/
 * 0x801F48A4. The world04/area160 entry-2 sibling at 0x801F2EF8 advances the
 * same halfword by its own step and draws with the same shape, so this name
 * follows that family.
 *
 * Shape: no other payload holds a byte-identical copy, because the trailing
 * `jal` names this overlay's draw routine, so the body is seeded from the
 * original bytes alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By4AndClearWorkFlagsAt100ThenDrawWorld02Area11113_801F31F0(
    void) {
  u8* work;
  u16 half;

  work = SPAD_PTR_SLOT(u8, 0x44);
  half = *(u16*)(work + 0x30) + 4;
  *(u16*)(work + 0x30) = half;
  if ((s16)half >= 0x100) {
    func_80196070();
  }
  func_801F3244();
}
