#include "internal.h"

/* @source 0x801E2120
 * @behavior Sets battle global flag 0x4 and dispatches the update when the scratchpad gate is clear.
 */
void battle03_raise_flag4_and_update(void) {
    u16* global_half;

    if (!(SPAD_PTR_SLOT(volatile u8, 0x44u)[0] & 0x40)) {
        global_half = (u16*)&BATTLE_GLOBAL_HALF_62E8;
        *global_half |= 4;
        func_801E1DD4();
    }
}
