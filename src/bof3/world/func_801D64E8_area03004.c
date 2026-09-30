#include "bof3/world/area03004_internal.h"

/* @behavior decrements byte 9 of the AREA030 work record published at the
 * scratchpad cursor 0x1F800044 and dispatches on the remaining count: a zero
 * count runs the shared func_80196070 helper, otherwise the shared func_8014D4E0
 * helper runs when bit 3 of that count is set.
 * @source 0x801D64E8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D64E8(void) {
    u8 *work;
    u8 *reload;

    work = D_1F800044;
    work[9] = work[9] - 1;
    reload = D_1F800044;
    if (reload[9] == 0) {
        func_80196070();
    } else if (reload[9] & 8) {
        func_8014D4E0();
    }
}
