#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D02F4: twenty-eight code pointers, entry 0
 * = 0x801CE448 through entry 27 = 0x801D025C, holding this payload's functions
 * plus the shared main-executable handler 0x801B2D54 at entries 2, 8 and 21.
 * The Splat `textbin` row T_801D02F4 places the words, so the run is only
 * referenced here. This dispatcher indexes it from the interior word 0x801D0318,
 * entry 9, so the nineteen addresses it can reach are entry 9 = 0x801CF37C
 * through entry 27 = 0x801D025C, and its own address is entry 26 of that run,
 * the word 0x801CF338 at 0x801D035C. */
extern void (*D_801D0318[])(void); /* @source 0x801D0318 @kind data */

/* Seeded from this payload's 0x801CF12C, 0x801CE72C and 0x801D0050 bodies
 * (callWorkTable030Plchar,
 * callWorkTable0300Plchar and
 * callWorkTable0340Plchar), whose own lifts measured
 * exact. A payload comparison of this 68-byte span against each of them found one
 * differing word: the low half of the scaled table load at offset 0x24, 0x0318
 * here against 0x030C, 0x0300 and 0x0340 there, because each dispatcher names the
 * base of its own run and every base sits in the one 0x801D page, so the `lui`
 * of that address stays equal. Statement order, the macro spelling and the
 * declaration style were carried over with this function's own table symbol and
 * name. */
/* @source 0x801CF338
 * @behavior Overlay step dispatch on the work record unsigned byte at offset
 *           0x02 from the interior base 0x801D0318, entry 9 of this overlay
 *           twenty-eight-word handler run at 0x801D02F4: loads the work-record
 *           pointer published at the scratchpad pointer cell 0x1F800044 once,
 *           takes that record unsigned byte at offset 2, scales it by four and
 *           calls the matching entry of the run based at 0x801D0318 with no
 *           arguments, discarding any result. The original does not bound the
 *           index before the scaled table load, and the entry at that base is
 *           this payload byte-3 dispatcher
 *           dispatchWorkByte3Table0320PlcharPlp03400_801CF37C. It reads no other
 *           state and writes none; the pointer-cell load precedes frame creation,
 *           the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 *           and the slots after the index byte and after the table load stay
 *           nops. Its address appears once in the payload, as the word 0x801CF338
 *           at 0x801D035C, entry 26 of the twenty-eight-word run and index 17 of
 *           the run based at 0x801D0318, so no jal in this payload names it; the
 *           overlays enter the body itself from outside the payload. It takes no
 *           arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callWorkTable0318Plchar, D_801D0318);
