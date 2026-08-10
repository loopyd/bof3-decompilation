#include "bof3/world/area03004_internal.h"

/* @behavior clears the scratch work-record action byte after setting its
 * state byte when both area flags are zero.
 * @source 0x801DAE84
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801DAE84(void) {
    u8 state;

    if (D_80149332 != 0) {
        return;
    }
    state = 1;
    if (modeByte != 0) {
        return;
    }
    D_1F800044[2] = state;
    D_1F800044[3] = 0;
}
