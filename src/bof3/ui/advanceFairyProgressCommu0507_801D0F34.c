#include "bof3/bof3.h"

extern u8 fairyProgress[]; /* @source 0x801448EC @kind data */

/* @source 0x801D0F34
 * @behavior Advances the shared fairy progress byte at 0x801448EC by one
 * through the array base and returns no value. The byte is the front-end fairy
 * progress counter the commu overlay handlers advance; this function reads and
 * writes nothing else and takes no arguments. Its only in-image reference is
 * word 2 of this overlay's run at 0x801D1950 (0x801D1958), which the overlay's
 * step dispatcher indexes with the scratchpad byte at 0x1F80294C and calls with
 * no arguments and no used result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceFairyProgressCommu0507_801D0F34(void) {
    fairyProgress[0] += 1;
}
