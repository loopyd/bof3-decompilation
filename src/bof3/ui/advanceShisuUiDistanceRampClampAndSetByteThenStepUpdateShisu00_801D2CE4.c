#include "bof3/bof3.h"

/* Shared drawing step of this overlay, still an `asm` row of its Splat layout,
 * so only its call is bound here and its own role stays unreviewed.
 * @source 0x801D2978
 */
extern void func_801D2978(void);

/* The shared distance helper: it takes the two signed coordinates the shared
 * work block holds at 0x80149308 and 0x8014930C and answers with their signed
 * distance, which this handler shifts left 16 before it compares. No payload of
 * this overlay defines it, so only its call is bound here.
 * @source 0x8015477C
 */
extern s32 func_8015477C(s32 x, s32 y);

/* The source position pair of the UI work block: both cells lie outside this
 * overlay's payload, so they belong to the shared image; the second is already
 * bound by the shared map, the first carries only this origin tag.
 */
extern s32 D_80149308; /* @source 0x80149308 @kind data */
extern s32 D_8014930C; /* @source 0x8014930C @kind data */

/* The offset 0x3C cell of the first UI work record based at 0x801DA048, a
 * 16.16 distance that steps toward the shared distance: this handler raises it
 * by 0x200000 and clamps it at the distance shifted left 16, and 0x801D27BC
 * seeds it to that same shifted distance lowered by 0x2000000. Those two bodies
 * are its only references in this payload. */
extern s32 D_801DA084; /* @source 0x801DA084 @kind data */

/* The overlay's one-byte step report at offset 6 of the same record: this
 * handler and the handler at 0x801D2C84 are its only two writers in this
 * payload, and each of them stores 1 on one arm of its test and 0 on the
 * other. */
extern u8 D_801DA04E; /* @source 0x801DA04E @kind data */

/* @source 0x801D2CE4
 * @behavior Step handler of the SHISU overlay, the state-3 entry of its step
 *           run, that raises one work word toward the shared distance, reports
 *           whether it reached it, and then runs the shared step. It takes the
 *           signed word at 0x801DA084, the offset 0x3C cell of the UI work
 *           record based at 0x801DA048 that 0x801D27BC seeds to the shared
 *           distance shifted left 16 and lowered by 0x2000000, adds 0x200000
 *           and stores the sum back in the delay slot of its first call. That
 *           call is the shared distance helper func_8015477C with the two
 *           shared coordinate cells 0x80149308 and 0x8014930C; when its answer
 *           shifted left 16 lies below the incremented word, this handler
 *           replaces the word with that shifted answer and sets the byte
 *           0x801DA04E to one, and otherwise it leaves the word alone and
 *           clears that byte. It then calls the shared drawing step
 *           func_801D2978 with no arguments. Its only in-image reference is the
 *           word 0x801D2CE4 at 0x801D972C, entry 3 of the step handler run
 *           based at 0x801D9720 that the dispatcher at 0x801D2C0C indexes from
 *           the state byte 0x801DA049, and no jal in this payload names it. Its
 *           0x18-byte frame keeps $s0 at 0x10 and $ra at 0x14, and the epilogue
 *           leaves the callee result in $v0 untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceShisuUiDistanceRampClampAndSetByteThenStepUpdateShisu00_801D2CE4(void)
{
    s32 *ramp = &D_801DA084;

    *ramp += 0x200000;
    if ((func_8015477C(D_80149308, D_8014930C) << 16) < *ramp) {
        *ramp = func_8015477C(D_80149308, D_8014930C) << 16;
        D_801DA04E = 1;
    } else {
        D_801DA04E = 0;
    }

    func_801D2978();
}
