#include "internal.h"

/* @source 0x801EA848
 * @behavior Advances the active panel state and raises its update flag when two gate bytes differ.
 */
void advancePanelWhenRingDiffers(void) {
    if (uiRingTail != uiRingHead) {
        ((u8*)D_80148648)[3]++;
        D_801EC2E4 = 1;
    }
}
