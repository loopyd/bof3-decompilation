#include "bof3/bof3.h"

/* Work-table stepper of this payload, lifted as
 * stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624:
 * it walks the 0x10 entries of
 * six-byte stride of the work table at 0x800E4800, and for every entry whose
 * lead byte is non-zero it advances that entry's state byte at +0x2 - on the
 * state-0 arm it claims the entry by writing 4 to the entry bytes at +0x1 and
 * +0x3, its argument to the halfword at +0x4 and 1 to the state byte, on
 * state 1 it increments the state byte, and on state 2 it subtracts 8 from
 * that halfword and clears the lead byte once the result is negative.
 * It returns non-zero in the frames where at least one entry lead byte read
 * non-zero. Both in-payload call sites pass the literal 0xF0, so the argument
 * and the result are declared as full words and only the low byte of the result
 * is consumed. */
s32 stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624(
    s32 entry_value);

/* Shared work-area reset helper of the game image: it zeroes work-object bytes
 * 0x00-0x04, including the work byte this overlay dispatches on, so the overlay
 * drops back to work state 0. No shipped header declares it, so the prototype
 * stays local, exactly as in the sibling lifts of the same helper. */
void func_80196070(void);

/* @source 0x801D0EC4
 * @behavior Entry 2 of the overlay step handler run at 0x801D1B2C, which is
 * word 6 of the fourteen-word code-pointer run at 0x801D1B1C and which
 * dispatchWorkByte1HandlerScenarioSce15ef200_801D0DA4 selects with the unsigned
 * work byte at offset +0x1 of the scratchpad work object published at pointer
 * slot 0x1F800044: it runs the work-table stepper
 * stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624
 * with the
 * literal 0xF0 (the value that stepper stores as the halfword at +0x4 of every
 * entry it claims in this frame) and, when that stepper masked to a byte
 * reports zero - that is, when no entry of the 0x800E4800 work table had a
 * non-zero lead byte - it resets the shared work-area block through
 * func_80196070, which zeroes work-object bytes 0x00-0x04 including that
 * dispatch byte. The call is reached by the run's dispatcher and by no `jal` in
 * this payload, so the overlay loader outside the payload enters it. It takes no
 * arguments, returns nothing, reads and writes no other state, and the
 * 0x18-byte frame only keeps $ra at 0x10 across the two calls; all three
 * instruction delay slots of the calls are filled by the argument setup or by
 * nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkTableValueF0AndResetWorkStateWhenNoRecordActiveScenarioSce15ef200_801D0EC4(void) {
  if ((stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624(0xF0) &
       0xFF) == 0) {
    func_80196070();
  }
}
