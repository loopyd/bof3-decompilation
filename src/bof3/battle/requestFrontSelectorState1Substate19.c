#include "bof3/battle/battle03_internal.h"

/* @source 0x801E5768
 * @behavior When the shared battle state byte 0x80146328 has bit 0 set and the
 * front-end gate halfword 0x80143F04 reads 0xBD, requests front-selector
 * primary state 1 with secondary sub-state 0x19 in the shared bytes
 * 0x80146874/0x80146875, then runs the shared front-end startup helper
 * func_8015C088 and initializes the shared 5/3/0 mode tuple through
 * initModeTuple530; otherwise it does nothing. Registered as the battle hook
 * D_801463A4 by func_801E320C and invoked from the battle step func_801D6A14.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestFrontSelectorState1Substate19(void) {
    if ((D_80146328 & 1) != 0 && D_80143F04 == 0xBD) {
        D_80146874 = 1;
        D_80146875 = 0x19;
        func_8015C088();
        initModeTuple530();
    }
}
