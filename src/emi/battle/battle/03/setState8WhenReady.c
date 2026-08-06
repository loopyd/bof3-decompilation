#include "internal.h"

/* @source 0x801E1D88
 * @behavior Calls localReadyOrHelper2 and, when nonzero, sets local-work bytes +0x01 to 8 and +0x02 to 0.
 */
void setState8WhenReady(void) {
    if (localReadyOrHelper2()) {
        D_1F800044->unk_01 = 8;
        D_1F800044->unk_02 = 0;
    }
}
