#include "bof3/battle/battle03_internal.h"

/* @source 0x801E72A8
 * @behavior selects an indirect handler from a stack-resident table using byte 1
 * from the non-volatile scratchpad pointer cell at 0x1F800044, then invokes it.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchByte1PairTable(void)
{
    Battle03Handler handlers[2];

    handlers[0] = func_801E72F4;
    handlers[battleWork[1]]();
}
