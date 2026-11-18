#include "bof3/bof3.h"
#include "bof3/plchar/work_byte_b_step.h"

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

/* Shared resource request helper of the shared main executable, outside this
 * payload: the plp016 and plp026 lifts of this family declare the same prototype
 * and pass a resource id, which is what this body passes from work byte 0x08.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the shared main executable, outside this
 * payload; the sibling lifts of this overlay family call it the same way and
 * discard any result.
 * @source 0x8014D978
 */
extern void func_8014D978(void);

/* @source 0x801CF37C
 * @behavior Work-step handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload bytes, at
 *           word 0x801CF938, which is index 2 of the byte-2 dispatch table based
 *           at 0x801CF930 and index 5 of the table based at 0x801CF924, both
 *           windows of the twenty-one-word handler run based at 0x801CF900; no
 *           jal in this payload names it, it takes no arguments and it answers
 *           no value. The body dispatches on the unsigned work byte 0x0B and
 *           keeps one done flag across the dispatch, which is why that flag
 *           survives the calls in a callee-saved register. When byte 0x0B reads
 *           one the body polls the shared readiness helper func_8014DAEC and,
 *           once that helper answers nonzero, passes the unsigned work byte 0x08
 *           to the shared resource request helper func_8014D6B8 and writes 2
 *           into byte 0x0B so the next dispatch takes the third arm; this arm
 *           never reports done. For every other nonzero value the body runs the
 *           shared no-argument update helper func_8014D978 and reports done
 *           whenever the state byte 0x80143BB0 reads zero, so a busy state of
 *           that byte suppresses the clear. When byte 0x0B reads zero the body
 *           only polls the readiness helper again and reports done as soon as
 *           that helper answers nonzero. All three arms leave their answer in
 *           the one flag, which is tested once: when it is set the body clears
 *           work byte 0x2B and then the state byte 0x12B of the front-end record
 *           published at 0x80146250, re-reading the pointer cell before each of
 *           the two stores. The roles of work byte 0x0B, of work byte 0x2B, of
 *           the requested resource and of that state byte stay
 *           address-anchored. The 0x18-byte frame only keeps $ra at 0x14 and
 *           the flag register at 0x10 across the calls, and every arm joins on
 *           one epilogue. The 0xEC-byte span is identical word for word to the
 *           plp247 overlay span at 0x801CF578 once the two absolute jump
 *           immediates of the shared tail label are masked, and those two words
 *           differ by exactly the 0x1FC offset between the two spans, so the two
 *           are one body; the plp247 lift gated exact on that statement order and
 *           uses the same shared macro this seed does, so the copy is shape
 *           evidence rather than a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
PLCHAR_DEFINE_WORK_BYTE_B_STEP(
    stepWorkByteBAndClearWorkByte2BWithRecordState12BWhenStepReportsDonePlcharPlp45700_801CF37C,
    D_1F800044, D_80146250, D_80143BB0,
    func_8014DAEC, func_8014D6B8, func_8014D978)
