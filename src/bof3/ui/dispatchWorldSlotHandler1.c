#include "bof3/ui/game00_internal.h"

/*
 * @behavior Calls handler 1 of the world-slot record that func_8019A194
 * selects by the current world state ID from the eleven 0x1C-byte records at
 * 0x801C7F5C.
 * @source 0x8019A258
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorldSlotHandler1(void)
{
    D_801C7F5C[func_8019A194()].handlers[1]();
}
