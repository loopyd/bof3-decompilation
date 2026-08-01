#include "internal.h"

/* @source 0x801E1D88
 * @behavior Calls func_801DEE4C and, when nonzero, sets local-work bytes +0x01 to 8 and +0x02 to 0.
 */
void func_801E1D88(void) {
    if (func_801DEE4C()) {
        D_1F800044->unk_01 = 8;
        D_1F800044->unk_02 = 0;
    }
}
