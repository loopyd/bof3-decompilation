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

/* Shared main-executable resource request helper of the shared main executable,
 * outside this payload: the sibling plp016 and plp026 lifts declare the same
 * prototype and pass a resource id, which is what this body passes from work
 * byte 0x08.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the shared main executable, outside this
 * payload; the sibling lifts of this overlay family call it the same way and
 * discard any result.
 * @source 0x8014D978
 */
extern void func_8014D978(void);

/* @source 0x801CF578
 * @behavior Work-step handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload bytes, at
 *           word 0x801CFB30, which is entry 13 of the twenty-word handler run
 *           based at 0x801CFAFC, and the neighbouring entries of that run hold
 *           the other steps of the same work object, so no jal in this payload
 *           names it, it takes no arguments and it answers no value. The body
 *           dispatches on the unsigned work byte 0x0B and keeps one done flag
 *           across the dispatch, which is why that flag has to survive the calls
 *           in a callee-saved register. When byte 0x0B reads one the body polls
 *           the shared readiness helper func_8014DAEC and, once that helper
 *           answers nonzero, passes the unsigned work byte 0x08 to the shared
 *           resource request helper func_8014D6B8 and writes 2 into byte 0x0B so
 *           the next dispatch takes the third arm; this arm never reports done.
 *           For every other nonzero value the body runs the shared no-argument
 *           update helper func_8014D978 and reports done whenever the state byte
 *           0x80143BB0 reads zero, so a busy state of that byte suppresses the
 *           clear. When byte 0x0B reads zero the body only polls the readiness
 *           helper again and reports done as soon as that helper answers
 *           nonzero. All three arms leave their answer in the one flag, which is
 *           tested once: when it is set the body clears work byte 0x2B and then
 *           the state byte 0x12B of the front-end record published at 0x80146250,
 *           re-reading the pointer cell before each of the two stores. The roles
 *           of work byte 0x0B, of work byte 0x2B, of the requested resource and
 *           of that state byte stay address-anchored. The 0x18-byte frame only
 *           keeps $ra at 0x14 and the flag register at 0x10 across the calls, and
 *           every arm joins on one epilogue. The 0xEC-byte span is identical word
 *           for word to seventeen other plchar overlay spans once the two
 *           absolute jump immediates of the shared tail label are masked, and the
 *           six of those that already have sources are the exact lifts of the
 *           plp012, plp015, plp016, plp025, plp026 and plp27a overlays, so this
 *           seed reuses their proven statement order, operand forms and
 *           declaration style; the byte difference is exactly those two jump
 *           words, which follow each overlay's own address, so those copies are
 *           shape evidence rather than clones.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDonePlcharPlp24700_801CF578(
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
