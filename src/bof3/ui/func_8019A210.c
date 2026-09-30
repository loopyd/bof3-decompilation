#include "bof3/ui/game00_internal.h"

/*
 * @behavior Calls handler 0 of the world-slot record that func_8019A194
 * selects by the current world state ID from the eleven 0x1C-byte records at
 * 0x801C7F5C; the sibling dispatchers 0x8019A258/0x8019A2A0/0x8019A2E8/
 * 0x8019A370 call handlers 1..4 of the same record.
 * @source 0x8019A210
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8019A210(void)
{
    D_801C7F5C[func_8019A194()].handlers[0]();
}
