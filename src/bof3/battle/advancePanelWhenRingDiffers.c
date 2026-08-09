#include "bof3/battle/battle03_internal.h"

/* @source 0x801EA848
 * @behavior Advances the active panel state and raises its update flag when two gate bytes differ.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advancePanelWhenRingDiffers(void) {
    if (uiRingTail != uiRingHead) {
        ((u8*)D_80148648)[3]++;
        D_801EC2E4 = 1;
    }
}
