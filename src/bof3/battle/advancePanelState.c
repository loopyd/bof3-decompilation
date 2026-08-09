#include "bof3/battle/battle03_internal.h"

/* @source 0x801EA728
 * @behavior loads panel state at offset 6; if state equals 0x12 increments byte
 * +3, otherwise adds 8 to state.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advancePanelState(void) {
    u8* panel = (u8*)D_80148648;
    s16 state = FIELD_REF(s16, panel, 6);

    if (state == 0x12) {
        FIELD_REF(u8, panel, 3)++;
    } else {
        FIELD_REF(s16, panel, 6) = state + 8;
    }
}
