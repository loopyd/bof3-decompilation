#include "internal.h"

/* @source 0x801EA848
 * @behavior Advances the active panel state and raises its update flag when two gate bytes differ.
 */
void battle03_panel_advance_when_ring_differs(void) {
    if (battle03UiRingTail != battle03UiRingHead) {
        ((u8*)D_80148648)[3]++;
        D_801EC2E4 = 1;
    }
}
