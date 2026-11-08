#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The body
 * reads it once for the byte it dispatches on and re-reads it before every
 * store, because a store through the pointer may alias the cell itself.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The body clears
 * byte 0x12B of the record it publishes when the step reports done.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Local state byte of the shared main executable: the front-end handlers that
 * own it treat 2 as busy and every other value as free. The shared symbol map of
 * this repository already names it.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Shared readiness helper of the shared main executable, outside this payload:
 * it takes no arguments and answers the ready byte this body tests for zero on
 * both polling arms, so the returned byte is kept as the answer.
 * @source 0x8014DAEC
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper, outside this payload: the
 * family siblings declare the same prototype and pass a resource id, which is
 * what this body passes from work byte 0x08.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the shared main executable, outside this
 * payload; the sibling lifts of this overlay family call it the same way and
 * discard any result.
 * @source 0x8014D978
 */
extern void func_8014D978(void);

/* @source 0x801CED80
 * @behavior Work-step handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address is the word 0x801CF6F8, entry 8 of the
 *           twenty-entry code pointer run based at 0x801CF6D8, which this
 *           target's Splat `textbin` row T_801CF6D8 places, so it is local index
 *           2 of the interior base 0x801CF6F0 that this payload's byte-2
 *           dispatcher dispatchWorkByte2Tablef6f0 indexes. No instruction of
 *           this payload names the body, but that dispatcher scales work byte 2
 *           by four without bounding it, so a work byte 2 of 2 reaches this
 *           body once the overlay loader has installed the run. It takes no
 *           arguments and answers no value. The body dispatches on the unsigned
 *           work byte 0x0B and keeps one done flag across the dispatch, which is
 *           why that flag has to survive the calls in a callee-saved register.
 *           When byte 0x0B reads one the body polls the shared readiness helper
 *           func_8014DAEC and, once that helper answers nonzero, passes the
 *           unsigned work byte 0x08 to the shared resource request helper
 *           func_8014D6B8 and writes 2 into byte 0x0B so the next dispatch takes
 *           the third arm; this arm never reports done. For every other nonzero
 *           value the body runs the shared no-argument update helper
 *           func_8014D978 and reports done whenever the state byte 0x80143BB0
 *           reads zero, so a busy state of that byte suppresses the clear. When
 *           byte 0x0B reads zero the body only polls the readiness helper again
 *           and reports done as soon as that helper answers nonzero. All three
 *           arms leave their answer in the one flag, which is tested once: when
 *           it is set the body clears work byte 0x2B and then the state byte
 *           0x12B of the front-end record published at 0x80146250, re-reading
 *           the pointer cell before each of the two stores. The roles of work
 *           byte 0x0B, of work byte 0x2B, of the requested resource and of that
 *           state byte stay address-anchored. The 0x18-byte frame only keeps $ra
 *           at 0x14 and the flag register at 0x10 across the calls, and every
 *           arm joins on one epilogue.
 *
 * Evidence: the payload span 0x801CED80-0x801CEE6C is one complete function -
 * its scheduled `lui/lw` of the 0x1F800044 cell runs ahead of
 * `addiu $sp,-0x18; sw $ra,0x14($sp); sw $s0,0x10($sp)`, it holds four `jal`s
 * (0x8014DAEC three times from two arms, 0x8014D6B8 and 0x8014D978) and ends
 * with the single epilogue `lw $ra,0x14($sp); lw $s0,0x10($sp);
 * addiu $sp,$sp,0x18; jr $ra; nop`, so the next subsegment at 0x801CEE6C is a
 * separate body and no carve is needed. Its own literals - the dispatched byte
 * 0x0B, the committed value 2, the work byte 0x2B and the record state byte
 * 0x12B - are read from these original bytes.
 *
 * Name evidence: this body is word-identical to the exact lifts already
 * reviewed in eight sibling plchar overlays under the same name -
 * stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDonePlcharPlp01200_801CEC30,
 * ...Plp01500_801CEC30, ...Plp01600_801CEC30, ...Plp02500_801CEC30,
 * ...Plp02600_801CEC30, ...Plp24700_801CF578, ...Plp25700_801CF958 and
 * ...Plp27a00_801CF35C - so the established family wording for the dispatched
 * work step byte 0x0B, the two unconditional clears and the front-end record
 * state byte 0x12B is reused rather than re-derived. The remaining roles stay
 * address-anchored.
 *
 * Shape evidence: statement order, operand forms and declaration style come from
 * those eight exact siblings. The two spans are NOT byte-identical: comparing
 * 0x801CED80 against 0x801CF958 leaves exactly two differing words, the `j`
 * instructions at span offsets 0x68 and 0x8C, whose immediates are
 * 0x08073B8B here against 0x08073E81 there - both encode the overlay's own
 * absolute address of the shared done test label, and every other word of the
 * 0xEC-byte span matches. The difference is therefore a relocation, so the
 * sibling is shape evidence and not a byte-identical clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDonePlcharPlp47800_801CED80(
    void) {
  u8 step;
  u8 done;

  step = D_1F800044[0xB];
  done = 0;
  if (step != 0) {
    if (step == 1) {
      if (func_8014DAEC()) {
        func_8014D6B8(D_1F800044[8]);
        D_1F800044[0xB] = 2;
      }
    } else {
      func_8014D978();
      if (D_80143BB0 == 0) {
        done = 1;
      }
    }
  } else {
    if (func_8014DAEC()) {
      done = 1;
    }
  }
  if (done) {
    D_1F800044[0x2B] = 0;
    D_80146250[0x12B] = 0;
  }
}
