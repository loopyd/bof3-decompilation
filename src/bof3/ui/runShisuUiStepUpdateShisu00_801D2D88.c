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

/* @source 0x801D2D88
 * @behavior Step handler of the SHISU overlay that runs the shared step routine
 *           func_801D2978 with no handler-specific work of its own. Its only
 *           in-image reference is the word 0x801D2D88 at 0x801D9730, entry 4 of
 *           the step handler run based at 0x801D9720, and no jal in this
 *           payload names it, so it is reached through that run alone and is an
 *           overlay entry called from outside the payload. The body takes no
 *           arguments, reads and writes no memory of its own, and its 0x18-byte
 *           frame only keeps $ra at 0x10 across the call, so the call result is
 *           left in $v0 untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runShisuUiStepUpdateShisu00_801D2D88(void)
{
    func_801D2978();
}
