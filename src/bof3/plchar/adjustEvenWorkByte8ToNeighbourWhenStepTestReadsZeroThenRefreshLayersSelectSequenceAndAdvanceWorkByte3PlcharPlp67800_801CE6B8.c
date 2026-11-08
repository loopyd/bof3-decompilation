#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it before the
 * record-byte read of every block and again before each store that follows a
 * call, so the source keeps the named symbol form and evaluates it again at
 * every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Step test of this payload, still an `asm` row here. It takes no arguments,
 * reads the work-object pointer cell itself, takes the direction-vector entries
 * of the word tables at 0x80181B94/0x80181B98 indexed by work byte 8, offsets
 * the record position held at work +0x34/+0x38 by them and answers 0 or 1 after
 * the map-code test of its own body. Its 0x1A4-byte span is byte-identical to
 * the copy at 0x801CF3C0 of plp467 that measured exact when this seed was
 * written. This caller reads that answer as an unsigned byte and passes no
 * argument, so the $a0 left over from the pointer load above is dead at the
 * first call.
 */
u8 func_801CF1A4(void);

/* Layer-position refresh of this payload, still an `asm` row here. Its bytes
 * claim work byte 0x2B, draw the work reference positions and clear the claim
 * again when a probe reads above 64, and they are byte-identical to the span at
 * 0x801CF564 of plp467 that the exact lift of the same tail describes. The
 * exact sibling of this payload at 0x801CEF64 declares the same prototype from
 * the same `jal` form: nothing is set up in the argument registers, so it takes
 * no arguments, and this caller reads no result from $v0.
 */
void func_801CF348(void);

/* Work-sequence selector of the shared EXE image, outside this payload. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt.
 */
u32 func_8014D8D4(u8 arg0);

/* Seeded from the exact lift of the same body at plp467 0x801CE8D4, which
 * measured 100.00 before this seed was written, with the statement order of
 * this payload's own exact tail at 0x801CEF64 for the sequence select and the
 * two stores that follow it. This 0x100-byte span compared to that plp467 range
 * over the payload bytes with exactly three differing words, the two step-test
 * `jal` immediates at +0x28 and +0x58 and the refresh `jal` immediate at +0x8C,
 * and every one of them shifts by the same delta 0x21C between the two spans
 * while the remaining sixty-one words stay equal, so one body and one statement
 * order are proven while the byte difference leaves no clone. This function's
 * own callee addresses are written from its own bytes.
 */
/* @source 0x801CE6B8
 * @behavior Work byte 3 == 0 step of this overlay: the word 0x801CE6B8 at
 *           0x801D0578 is index 0 of the three-entry handler run that this
 *           payload byte-3 dispatcher 0x801CE674 indexes by work byte 3 scaled
 *           by four, and 0x801D0578 is entry 5 of the thirty-word code-pointer
 *           run based at 0x801D0564, so the body runs for work byte 3 == 0 and
 *           no jal in this payload names it. When work byte 8 is even it first
 *           rewrites that byte: it stores the decremented direction
 *           ((byte 8 - 1) & 7), calls the payload step test at 0x801CF1A4 and,
 *           only while that call answers zero, stores the neighbour two steps
 *           the other way ((byte 8 + 2) & 7) and calls the same test again; if
 *           that call answers zero too the byte is put back to the decremented
 *           direction. Both paths then run the payload layer-position refresh at
 *           0x801CF348, select the shared work sequence through func_8014D8D4
 *           with ((byte 8 - 1) / 2 + 0x42) truncated to a byte, clear work byte
 *           0xB, write 0xB into work byte 0xA, the countdown the byte-3 entry 1
 *           handler at 0x801CEFD0 decrements, and 1 into work byte 3, which
 *           advances the dispatch to that same run entry 1 at 0x801CE7B8. The
 *           pointer cell is loaded once before the frame is created for the
 *           first byte-8 read and store, and is re-loaded before the byte-8 read
 *           of each inner block and before each of the three stores after the
 *           calls. The subtraction leaves the division signed, so byte 8 == 0, 1
 *           and 2 all select sequence 0x42 while byte 8 == 3 gives 0x43. It
 *           takes no arguments, returns nothing, and the 0x18-byte frame only
 *           keeps $ra at 0x10 across the calls. The roles of work byte 8, of the
 *           selected sequence, of the step test and of the pointer cell contents
 *           are not established in this payload, so the name describes the
 *           effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adjustEvenWorkByte8ToNeighbourWhenStepTestReadsZeroThenRefreshLayersSelectSequenceAndAdvanceWorkByte3PlcharPlp67800_801CE6B8(
    void) {
  if (!(D_1F800044[8] & 1)) {
    D_1F800044[8] = (D_1F800044[8] - 1) & 7;
    if (func_801CF1A4() == 0) {
      D_1F800044[8] = (D_1F800044[8] + 2) & 7;
      if (func_801CF1A4() == 0) {
        D_1F800044[8] = (D_1F800044[8] - 2) & 7;
      }
    }
  }
  func_801CF348();
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xB] = 0;
  D_1F800044[0xA] = 0xB;
  D_1F800044[3] = 1;
}
