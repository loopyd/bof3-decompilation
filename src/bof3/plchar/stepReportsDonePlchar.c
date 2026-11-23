#include "bof3/bof3.h"
#include "bof3/plchar/work_byte_b_step.h"

/* Scratchpad work-object pointer cell of the shared main executable. This body
 * reads it once for the byte it dispatches on and re-reads it before each store,
 * because a store through the pointer may alias the cell itself.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record pointer cell of the shared main executable. The done path
 * clears byte 0x12B of the record published here; the role of that byte is not
 * established in this overlay, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Front-end phase byte of the shared main executable. The handlers of this
 * overlay store 2 into it to mark the phase busy, so the zero test here is a
 * free-phase test and the clear is suppressed while it reads busy. This target's
 * shared symbol map already names the byte, so no row is added for it.
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
 * payload: the sibling lifts of this target declare the same prototype and pass
 * a resource id, which is what this body passes from work byte 0x08.
 * @source 0x8014D6B8
 */
extern void func_8014D6B8(u32 flag);

/* Shared no-argument update helper of the shared main executable, outside this
 * payload; the sibling lifts of this target call it the same way and discard any
 * result.
 * @source 0x8014D978
 */
extern void func_8014D978(void);

/* @source 0x801CEC30
 * @behavior Work-byte-2 step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, entry 5 of the twenty-four-word
 *           code-pointer run based at 0x801D0858 that the step dispatcher
 *           func_801CE404 indexes by work byte 0x02: the run's sixth word is
 *           0x801CEC30 at 0x801D086C, and the run is the `textbin` row at payload
 *           offset 0x2458. The same word is entry 2 of the sub-run based at
 *           0x801D0864 that the dispatcher func_801CE72C indexes the same way.
 *           No jal immediate in this payload names the body, so the run is its
 *           only in-image reference, it takes no arguments and it answers no
 *           value. The body dispatches on the unsigned work byte 0x0B and keeps
 *           one done flag across the dispatch, which is why that flag has to
 *           survive the calls in a callee-saved register. When byte 0x0B reads
 *           one the body polls the shared readiness helper func_8014DAEC and,
 *           once that helper answers nonzero, passes the unsigned work byte 0x08
 *           to the shared resource request helper func_8014D6B8 and writes 2 into
 *           byte 0x0B so the next dispatch takes the third arm; this arm never
 *           reports done. For every other nonzero value the body runs the shared
 *           no-argument update helper func_8014D978 and reports done whenever the
 *           front-end phase byte 0x80143BB0 reads zero, so a busy value there
 *           suppresses the clear. When byte 0x0B reads zero the body only polls
 *           the readiness helper again and reports done as soon as that helper
 *           answers nonzero. All three arms leave their answer in the one flag,
 *           which is tested once: when it is set the body clears work byte 0x2B
 *           and then byte 0x12B of the record published at 0x80146250, re-reading
 *           the scratchpad cell before the first store and the record cell before
 *           the second. The roles of work bytes 0x0B, 0x2B and 0x08 and of
 *           record byte 0x12B stay address-anchored. The 0x18-byte frame only
 *           keeps $ra at 0x14 and the flag at 0x10 across the calls, the pointer
 *           cell is read before the frame is created, the flag is cleared in the
 *           delay slot of the first test, the constant 2 fills the delay slot of
 *           the jump to the shared done test, the flag of the other-nonzero arm
 *           is set in the delay slot of the jump that closes it while the zero
 *           arm sets it by fall-through, the done mask of the flag is folded into
 *           three branch delay slots, and every arm joins on one epilogue. The 0xEC-byte span at payload offset 0x830
 *           is byte-identical to the span at the same offset of the plp025 and
 *           plp026 payloads: one comparison of the three 0xEC-byte spans hashed
 *           each to 81eb438c601d3e6d0f71176c363a0f25ae4c0e09. The plp025 lift of
 *           that span measured exact and states the same behavior, and the shared
 *           header macro invoked below already backs it, so this source is that
 *           macro invoked with this target's own cells. The clone adapter refuses
 *           that lift as a representative because its whole body is the macro
 *           invocation it would have had to copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
PLCHAR_DEFINE_WORK_BYTE_B_STEP(
    stepReportsDonePlchar,
    D_1F800044, D_80146250, D_80143BB0,
    func_8014DAEC, func_8014D6B8, func_8014D978)
