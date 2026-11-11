#include "bof3/bof3.h"

/* Shared drawing step of this overlay, still an `asm` row of its Splat layout,
 * so only its call is bound here and its own role stays unreviewed. Its bytes
 * save the work bytes 0x801DA0A5 through 0x801DA0A7 on its own frame, raise
 * 0xA5 and 0xA6 by three times the step byte 0x801DA02E up to a ceiling of
 * 0xFF, and lower 0xA7 by the same amount down to a floor of zero. They then
 * store one word of the fixed-point table based at T_801D9688 into 0x801DA088
 * at the index that the signed difference of the work bytes 0x801DA02D and
 * 0x801DA030 clamped to 1 through 0x14 gives, call the overlay drawing routine
 * func_801D2308 with the work record at 0x801DA048, and write the three saved
 * bytes back before returning.
 * @source 0x801D2978
 */
extern void func_801D2978(void);

/* The overlay's step accumulator, part of the work block 0x801DA028..0x801DA12D:
 * func_801D27BC clears it while setting up that block, this handler at
 * 0x801D2C50 raises it by 0x20 per step, and the handler at 0x801D2C84 reads it
 * back, keeps bits 6 through 11 of it and tests bits 0 through 11 before it sets
 * or clears the byte 0x801DA04E. Those three bodies are its only references in
 * this payload. */
extern u32 D_801DA0B4; /* @source 0x801DA0B4 @kind data */

/* @source 0x801D2C50
 * @behavior Step handler of the SHISU overlay that advances one work word and
 *           then runs the shared step: it takes the unsigned word at 0x801DA0B4,
 *           adds 0x20 to it and stores it back in the call's delay slot before
 *           calling the shared drawing step func_801D2978 with no arguments. Its
 *           only in-image reference is the word 0x801D2C50 at 0x801D9724, entry
 *           1 of the step handler run based at 0x801D9720, and no jal in this
 *           payload names it, so it is reached through that run alone and is an
 *           overlay entry called from outside the payload. It reads no other
 *           state and writes none besides that word, its 0x18-byte frame only
 *           keeps $ra at 0x10 across the call, and the epilogue leaves the
 *           callee's result in $v0 untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceShisuUiWordThenStepUpdateShisu00_801D2C50(void)
{
    u32 *workWord = &D_801DA0B4;

    *workWord += 0x20;
    func_801D2978();
}
