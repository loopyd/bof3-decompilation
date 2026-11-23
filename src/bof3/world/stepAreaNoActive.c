#include "bof3/bof3.h"

u32 func_801F2EC4(void);
void func_80196070(void);

/* @source 0x801F2E28
 * @behavior Entry 3 of this overlay's work-byte-1 state machine: entry 3 of the
 *           eight-word handler run based at 0x801F4FAC, which this target's
 *           exact dispatcher dispatchStateByte1Table4facWorld04Area17413_801F2C04
 *           indexes with the work record's state byte at +1. That base sits
 *           inside the payload textbin blob, so the third word of the run
 *           (0x801F4FAC + 3*4) names this span and no jal in the payload
 *           reaches it. Entry 2 at 0x801F2D68 advances that state byte once the
 *           resident position word reaches 0x408000, so the next frame
 *           dispatches here.
 *           It runs the in-payload record stepper at 0x801F2EC4, which walks
 *           the 0x20 records of the 0x2C-byte-stride table at 0x800E4800,
 *           integrates the three position words of every claimed record from
 *           its step triple, reseeds the heading, retires the records whose
 *           countdown at +5 reached zero or whose first position word fell
 *           behind the resident position, and answers 1 when any record of the
 *           table read claimed during the pass and 0 when the whole table read
 *           unclaimed. Only when the low byte of that answer reads zero does it
 *           call the resident reset helper at 0x80196070 - the main-executable
 *           body that target binds as clearWorkFlags, which zeroes the work
 *           object's bytes 0x00 through 0x04 - so the state byte at +1 that
 *           selects this entry clears as well and the machine restarts at entry
 *           0.
 *           The state byte therefore stays at 3 while any record remains, and
 *           this entry repeats every frame. It takes no arguments and returns
 *           nothing; the 0x18-byte frame only keeps $ra at 0x10 across the two
 *           calls, both join on the single epilogue, and the word answer of the
 *           stepper is reduced to its low byte before the test.
 *
 * Name evidence: the stepper call and the low-byte test on its answer give the
 *           record-stepping role and the "no record active" condition, and the
 *           reset helper reached only on that condition gives the work-area
 *           reset. Its position in the 0x801F4FAC run comes from the payload
 *           word at that base, and the run's dispatcher and byte index come from
 *           the two exact neighbours at 0x801F2C04 and 0x801F2C48.
 *
 * Shape evidence: no byte-identical sibling exists - a scan of every payload
 *           under out/binaries found the 0x34-byte span only at offset 0x228 of
 *           this target's own binary - so the clone lever is declined. The
 *           statement order and the prototype style follow the two exact
 *           in-target neighbours at 0x801F2CA8 and 0x801F2D68, which call the
 *           same stepper, and the low-byte test spelling
 *           `(callee() & 0xFF) == 0` plus the empty delay slots of both call
 *           words were proved by the exact cross-target span at 0x801F2CC8 of
 *           emi/world02/area108/14, whose original bytes carry the same mask and
 *           branch pair.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepAreaNoActive(
    void) {
  if ((func_801F2EC4() & 0xFF) == 0) {
    func_80196070();
  }
}
