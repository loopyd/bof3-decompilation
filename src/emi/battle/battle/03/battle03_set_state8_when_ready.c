#include "internal.h"

/* @source 0x801E1D88
 * @behavior Calls battle03_local_ready_or_helper2 and, when nonzero, sets local-work bytes +0x01 to 8 and +0x02 to 0.
 */
void battle03_set_state8_when_ready(void) {
    if (battle03_local_ready_or_helper2()) {
        D_1F800044->unk_01 = 8;
        D_1F800044->unk_02 = 0;
    }
}
