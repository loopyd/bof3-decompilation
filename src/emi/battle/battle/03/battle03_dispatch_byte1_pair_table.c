#include "internal.h"

/* @source 0x801E72A8
 * @behavior selects an indirect handler from a stack-resident table using byte 1
 * from the non-volatile scratchpad pointer cell at 0x1F800044, then invokes it.
 */
void battle03_dispatch_byte1_pair_table(void)
{
    Battle03Handler handlers[2];

    handlers[0] = func_801E72F4;
    handlers[g_battle03_work[1]]();
}
