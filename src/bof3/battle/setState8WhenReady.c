#include "bof3/battle/battle03_internal.h"

/* @source 0x801E1D88
 * @behavior Calls localReadyOrHelper2 and, when nonzero, sets local-work bytes +0x01 to 8 and +0x02 to 0.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void setState8WhenReady(void) {
    if (localReadyOrHelper2()) {
        D_1F800044->unk_01 = 8;
        D_1F800044->unk_02 = 0;
    }
}
