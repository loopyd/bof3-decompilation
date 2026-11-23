#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: seventeen code pointers, the words
 * 0x801CF900 through 0x801CF940, entry 0 = 0x801CE448 through entry 16 =
 * 0x801CF868, holding this payload's own step handlers plus the shared
 * main-executable handler 0x801B2D54 at entries 2 and 7. The reviewed Splat
 * layout places all seventeen words in the one `textbin` row `codePointerTablef900`, so the
 * run is only referenced here. This dispatcher indexes it from 0x801CF920, so its
 * base is entry 8 and the nine entries it can reach run from 0x801CEE74 (entry 8)
 * through 0x801CF868 (entry 16). */
extern void (*D_801CF920[])(void); /* @source 0x801CF920 @kind data */

/* Seeded from the exact lift of the same body in this target at 0x801CE404
 * (callWorkTablef900Plchar): `cmp` of the two 68-byte
 * payload spans reports one differing byte, offset 0x24, the low byte of the
 * table load's `%lo` immediate - 0x00 there for base 0x801CF900, 0x20 here for
 * 0x801CF920. Both bases sit in the one 0x801D page, so that load's `%hi` word
 * and every other word stay equal. The target's other two dispatchers,
 * 0x801CE610 and 0x801CEBFC, carry the same single-immediate difference at their
 * own bases. The body is therefore the reviewed shared dispatcher shape, so the
 * lift writes the macro invocation and this function's own table base rather
 * than a transcribed statement list. */
/* @source 0x801CEE30
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           before the frame is created, takes that record's unsigned byte at
 *           offset 2, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CF920 with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load. It reads no other state and writes none; the
 *           0x18-byte frame only keeps $ra at 0x10 across the indirect call, and
 *           the slot after the index byte and the slot after the table load stay
 *           nops. No jal of this payload names the body; its address appears
 *           once, as the word 0x801CEE30 at 0x801CF93C, entry 15 of the
 *           seventeen-word run at 0x801CF900, so the overlay loader enters the
 *           body from outside the payload while this body's own base also
 *           dispatches to it at its index 7.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTablef920Plchar, D_801CF920);
