#include "bof3/bof3.h"

extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* @source 0x801D1628
 * @behavior Reads the shared frontend mode byte at 0x80143BB0 and returns while
 *           it already equals 2; otherwise starts action 0x63 through
 *           func_80150224 and latches the same mode byte to 2, returning no
 *           value. The byte is the only state it reads or writes and it takes no
 *           arguments. Its only in-image reference is the last word of this
 *           overlay's handler run at 0x801D1750, which the step dispatcher
 *           0x801D0C5C indexes with the scratchpad byte 0x801F294D and calls
 *           with no arguments and no used result, so it has no jal caller. The
 *           address of the mode byte stays in $s0 across the branch and the
 *           call, so the local pointer is seeded before the branch as written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startAction63AndSetFrontendMode2(void) {
    u8* state = &D_80143BB0;

    if (*state == 2) {
        return;
    }
    func_80150224(0x63);
    *state = 2;
}
