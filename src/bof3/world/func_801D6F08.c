#include "bof3/world/area03004_internal.h"

/*
 * @behavior Seeds the menu scratch record, then stores 0x37 at cursor+0xB.
 * @source 0x801D6F08
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D6F08(void) {
    seedMenuScratch();
    D_1F800044[0xB] = 0x37;
}
