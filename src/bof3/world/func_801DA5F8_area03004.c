#include "bof3/world/area03004_internal.h"

/* @behavior increments the shared byte counter at 0x801440B1, calls
 * func_8014D8D4 with 1, then stores 1 in byte 3 of the work record
 * published at the scratchpad cursor 0x1F800044.
 * @source 0x801DA5F8
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA5F8(void) {
    u8 *counter = &D_801440B1;

    *counter = *counter + 1;
    func_8014D8D4(1);
    D_1F800044[3] = 1;
}
