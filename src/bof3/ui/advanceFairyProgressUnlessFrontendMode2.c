#include "bof3/bof3.h"

extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */
extern u8 fairyProgress[]; /* @source 0x801448EC @kind data */

/* @source 0x801D1670
 * @behavior Reads the shared frontend mode byte at 0x80143BB0 as a scalar and
 *           returns while it equals 2; otherwise advances the shared fairy
 *           progress byte at 0x801448EC by one through the array base and
 *           returns. It takes no arguments, reads no other state and writes
 *           nothing else. Its only in-image reference is word 1 of this
 *           overlay's handler run at 0x801D1750, which the overlay's step
 *           dispatcher 0x801D0C04 indexes with the scratchpad byte 0x801F294C
 *           and calls with no arguments and no used result, so it has no jal
 *           caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceFairyProgressUnlessFrontendMode2(void) {
    if (D_80143BB0 == 2) {
        return;
    }
    fairyProgress[0] += 1;
}
