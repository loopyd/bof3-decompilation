#include "bof3/bof3.h"

/* Shared drawing step of this overlay, still an `asm` row of its Splat layout,
 * so only its call is bound here and its own role stays unreviewed.
 * @source 0x801D2978
 */
extern void func_801D2978(void);

/* The overlay's step accumulator, part of the work block 0x801DA028..0x801DA12D:
 * initializeShisuUiWorkRecordsShisu00_801D27BC clears it while setting up that
 * block, the handler at 0x801D2C50 raises it by 0x20 per step, and this handler
 * reads it back and keeps only its low twelve bits. Those three bodies are its
 * only references in this payload. */
extern u32 D_801DA0B4; /* @source 0x801DA0B4 @kind data */

/* The overlay's one-byte step report, also part of the work block
 * 0x801DA028..0x801DA12D: this handler and the handler at 0x801D2CE4 are its
 * only writers in this payload, and each of them stores 1 on one arm of its test
 * and 0 on the other. No body of this payload reads it back. */
extern u8 D_801DA04E; /* @source 0x801DA04E @kind data */

/* @source 0x801D2C84
 * @behavior Step handler of the SHISU overlay that keeps the shared step word
 *           0x801DA0B4 on a 0x40 grid inside a 0x1000 window and reports the
 *           result through the byte 0x801DA04E before running the shared step.
 *           It loads that word, tests its low twelve bits and, when they are
 *           non-zero, adds 0x40 and masks the sum with 0xFC0 before storing it
 *           back and lowering the byte to zero; a word whose low twelve bits are
 *           already zero is left untouched and raises that byte to one. It then
 *           calls the shared drawing step func_801D2978 with no arguments. Its
 *           only in-image reference is the word 0x801D2C84 at 0x801D9728, entry
 *           2 of the step handler run based at 0x801D9720 that the dispatcher at
 *           0x801D2C0C indexes from the state byte 0x801DA049, and no jal in
 *           this payload names it. It reads no other state, its 0x18-byte frame
 *           only keeps $ra at 0x10 across the call, and the epilogue leaves the
 *           callee's result in $v0 untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void alignShisuUiWordToBoundaryAndSetByteThenStepUpdateShisu00_801D2C84(void)
{
    u32 *word = &D_801DA0B4;
    u32 value = *word;

    if ((value & 0xFFF) != 0) {
        *word = (value + 0x40) & 0xFC0;
        D_801DA04E = 0;
    } else {
        D_801DA04E = 1;
    }

    func_801D2978();
}
